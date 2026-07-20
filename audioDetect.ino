/*
 * audioDetect.ino
 *
 * BCLKおよびLRCK/DSDデータから、
 * PCM/DSDモードとサンプリングレートを検出する。
 */

// -----------------------------------------------------------------------------
// PCNT initialization
// -----------------------------------------------------------------------------

static void initPcntInput(pcnt_unit_t unit, int gpioPin)
{
  pcnt_config_t config = {};

  config.pulse_gpio_num = gpioPin;
  config.ctrl_gpio_num  = PCNT_PIN_NOT_USED;
  config.pos_mode       = PCNT_COUNT_INC;
  config.neg_mode       = PCNT_COUNT_DIS;
  config.lctrl_mode     = PCNT_MODE_KEEP;
  config.hctrl_mode     = PCNT_MODE_KEEP;
  config.counter_h_lim  = 32767;
  config.counter_l_lim  = 0;
  config.unit           = unit;
  config.channel        = PCNT_CHANNEL_0;

  pcnt_unit_config(&config);
  pcnt_filter_disable(unit);

  pcnt_counter_pause(unit);
  pcnt_counter_clear(unit);
}


void initAudioSignalDetector()
{
  initPcntInput(BCLK_PCNT_UNIT, AUDIO_BCLK_PIN);
  initPcntInput(LRCK_PCNT_UNIT, AUDIO_LRCK_PIN);

  detectedAudioMode = AUDIO_MODE_NONE;
  pendingAudioMode  = AUDIO_MODE_NONE;

  lastValidFS      = 0;
  pendingFS        = 0;
  audioStableCount = 0;

  measuredBclkHz     = 0;
  measuredLrckEdgeHz = 0;

  dsdOn = 0;
}


// -----------------------------------------------------------------------------
// Frequency measurement
// -----------------------------------------------------------------------------

static void measureAudioSignals(
  uint32_t measurementTimeUs,
  uint32_t *bclkHz,
  uint32_t *lrckEdgeHz)
{
  int16_t bclkCount = 0;
  int16_t lrckCount = 0;

  // 両カウンタを停止
  pcnt_counter_pause(BCLK_PCNT_UNIT);
  pcnt_counter_pause(LRCK_PCNT_UNIT);

  // 両カウンタをクリア
  pcnt_counter_clear(BCLK_PCNT_UNIT);
  pcnt_counter_clear(LRCK_PCNT_UNIT);

  // ほぼ同時に測定開始
  pcnt_counter_resume(BCLK_PCNT_UNIT);
  pcnt_counter_resume(LRCK_PCNT_UNIT);

  delayMicroseconds(measurementTimeUs);

  // ほぼ同時に測定終了
  pcnt_counter_pause(BCLK_PCNT_UNIT);
  pcnt_counter_pause(LRCK_PCNT_UNIT);

  pcnt_get_counter_value(
    BCLK_PCNT_UNIT,
    &bclkCount);

  pcnt_get_counter_value(
    LRCK_PCNT_UNIT,
    &lrckCount);

  if (bclkCount < 0) {
    bclkCount = 0;
  }

  if (lrckCount < 0) {
    lrckCount = 0;
  }

  measuredBclkCount = bclkCount;
  measuredLrckCount = lrckCount;

  /*
   * 先にuint64_tへキャストする。
   *
   * DSD256:
   *   5645 × 1,000,000 = 5,645,000,000
   *
   * 32bitではオーバーフローするため、
   * 64bitで乗算してから測定時間で割る。
   */
  *bclkHz =
    (uint32_t)(
      ((uint64_t)(uint16_t)bclkCount * 1000000ULL) /
      measurementTimeUs
    );

  *lrckEdgeHz =
    (uint32_t)(
      ((uint64_t)(uint16_t)lrckCount * 1000000ULL) /
      measurementTimeUs
    );
}


// -----------------------------------------------------------------------------
// Utility
// -----------------------------------------------------------------------------

static uint32_t absoluteDifference(uint32_t a, uint32_t b)
{
  return (a > b) ? (a - b) : (b - a);
}


// -----------------------------------------------------------------------------
// PCM rate detection
// -----------------------------------------------------------------------------

static uint16_t detectNearestPcmRate(uint32_t measuredHz)
{
  struct PcmRateEntry {
    uint32_t frequency;
    uint16_t fsrValue;
  };

  static const PcmRateEntry pcmRates[] = {
    {  32000,  32 },
    {  44100,  44 },
    {  48000,  48 },
    {  88200,  88 },
    {  96000,  96 },
    { 176400, 176 },
    { 192000, 192 },
    { 352800, 352 },
    { 384000, 384 }
  };

  uint32_t bestError = UINT32_MAX;
  uint32_t bestFrequency = 0;
  uint16_t bestFSR = 0;

  for (const auto &rate : pcmRates) {
    uint32_t error =
      absoluteDifference(measuredHz, rate.frequency);

    if (error < bestError) {
      bestError = error;
      bestFrequency = rate.frequency;
      bestFSR = rate.fsrValue;
    }
  }

  if (bestFrequency == 0) {
    return 0;
  }

  uint32_t allowedError = bestFrequency / 20;

/*
 * 500us測定では周波数換算値が2kHz単位になる。
 * 44.1kHzが44kHzまたは46kHzになる可能性があるため、
 * 最低許容誤差を2.5kHzとする。
 */
if (allowedError < 2500) {
  allowedError = 2500;
}

  if (bestError > allowedError) {
    return 0;
  }

  return bestFSR;
}


// -----------------------------------------------------------------------------
// DSD rate detection
// -----------------------------------------------------------------------------

static uint16_t detectNearestDsdRate(uint32_t measuredHz)
{
  struct DsdRateEntry {
    uint32_t frequency;
    uint16_t fsrValue;
  };

  static const DsdRateEntry dsdRates[] = {
    {  2822400,  2822 },
    {  5644800,  5644 },
    { 11289600, 11289 },
    { 22579200, 22579 }
  };

  uint32_t bestError = UINT32_MAX;
  uint32_t bestFrequency = 0;
  uint16_t bestFSR = 0;

  for (const auto &rate : dsdRates) {
    uint32_t error =
      absoluteDifference(measuredHz, rate.frequency);

    if (error < bestError) {
      bestError = error;
      bestFrequency = rate.frequency;
      bestFSR = rate.fsrValue;
    }
  }

  if (bestFrequency == 0) {
    return 0;
  }

  uint32_t allowedError = bestFrequency / 25;

  if (bestError > allowedError) {
    return 0;
  }

  return bestFSR;
}


// -----------------------------------------------------------------------------
// PCM / DSD classification
// -----------------------------------------------------------------------------

static AudioSignalMode classifyAudioSignal(
  uint32_t bclkHz,
  uint32_t lrckEdgeHz)
{
  if (bclkHz < 500000UL) {
    return AUDIO_MODE_NONE;
  }

  float edgeRatio =
    (float)lrckEdgeHz / (float)bclkHz;

  if (edgeRatio >= DSD_EDGE_RATIO_THRESHOLD) {
    return AUDIO_MODE_DSD;
  }

  return AUDIO_MODE_PCM;
}


// -----------------------------------------------------------------------------
// Detection stabilization
// -----------------------------------------------------------------------------

static uint16_t stabilizeAudioDetection(
  AudioSignalMode candidateMode,
  uint16_t candidateFS)
{
  /*
   * 無信号も候補の一つとして連続回数を数える。
   */
  if ((candidateMode == AUDIO_MODE_NONE) ||
      (candidateFS == 0)) {

    if ((pendingAudioMode == AUDIO_MODE_NONE) &&
        (pendingFS == 0)) {

      if (audioStableCount < AUDIO_DETECT_STABLE_COUNT) {
        audioStableCount++;
      }
    }
    else {
      pendingAudioMode = AUDIO_MODE_NONE;
      pendingFS = 0;
      audioStableCount = 1;
    }

    if (audioStableCount >= AUDIO_DETECT_STABLE_COUNT) {
      detectedAudioMode = AUDIO_MODE_NONE;
      dsdOn = 0;
      lastValidFS = 0;
    }

    return lastValidFS;
  }

  if ((candidateMode == pendingAudioMode) &&
      (candidateFS == pendingFS)) {

    if (audioStableCount < AUDIO_DETECT_STABLE_COUNT) {
      audioStableCount++;
    }
  }
  else {
    pendingAudioMode = candidateMode;
    pendingFS = candidateFS;
    audioStableCount = 1;
  }

  if (audioStableCount >= AUDIO_DETECT_STABLE_COUNT) {
    detectedAudioMode = candidateMode;
    lastValidFS = candidateFS;

    dsdOn =
      (detectedAudioMode == AUDIO_MODE_DSD) ? 1 : 0;
  }

  return lastValidFS;
}


// -----------------------------------------------------------------------------
// Main detection function
// -----------------------------------------------------------------------------

uint16_t detectFS()
{
  /*
   * BCLKとLRCK/DSDデータを同じ500usの時間窓で測定する。
   *
   * 最大49.152MHzの場合:
   *   49.152MHz × 500us = 24576カウント
   *
   * PCNTの上限32767以内に収まる。
   */
  measureAudioSignals(
    AUDIO_MEASUREMENT_US,
    &measuredBclkHz,
    &measuredLrckEdgeHz);

  /*
   * 今回の測定結果からPCM/DSD候補を判定する。
   *
   * PCM 352.8kHz、BCLK=128Fsの場合:
   *   352.8kHz / 45.1584MHz = 0.0078125
   *
   * したがって、閾値0.08より小さくPCM判定になる。
   */
  candidateAudioMode =
    classifyAudioSignal(
      measuredBclkHz,
      measuredLrckEdgeHz);

  candidateAudioFS = 0;

  if (candidateAudioMode == AUDIO_MODE_PCM) {
    candidateAudioFS =
      detectNearestPcmRate(measuredLrckEdgeHz);
  }
  else if (candidateAudioMode == AUDIO_MODE_DSD) {
    candidateAudioFS =
      detectNearestDsdRate(measuredBclkHz);
  }

  /*
   * 同じ候補が連続して検出された場合だけ、
   * detectedAudioModeとlastValidFSを更新する。
   */
  return stabilizeAudioDetection(
    candidateAudioMode,
    candidateAudioFS);
}
