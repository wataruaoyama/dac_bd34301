/*************************************************************
  ボリュームカウンタ
  チャタリング対策 + 長押しリピート付き

  タイマー割り込み周期: 10msec 前提

  volumeCounter:
    0   = 0.0dB
    220 = -110.0dB
    255 = -∞
 *************************************************************/
void volumeControl() {
  const uint8_t DEBOUNCE_COUNT = 1; // 1回連続で同じ状態なら確定

  // 10msec周期前提
  // 実機ではvolumeControl()の呼び出し周期が約50msec程度かな
  const uint8_t REPEAT_START_COUNT = 10;  // 感覚的には約500msec後から長押し開始
  const uint8_t REPEAT_NEXT_COUNT  = 2;   // 感覚的には約100msecごとに連続増減

  static boolean upStable   = HIGH;
  static boolean downStable = HIGH;

  static boolean upLastRaw   = HIGH;
  static boolean downLastRaw = HIGH;

  static uint8_t upDebounceCount   = 0;
  static uint8_t downDebounceCount = 0;

  static boolean upPrevStable   = HIGH;
  static boolean downPrevStable = HIGH;

  static uint8_t upHoldCount   = 0;
  static uint8_t downHoldCount = 0;

  static uint8_t upRepeatCount   = 0;
  static uint8_t downRepeatCount = 0;

  boolean upRaw   = digitalRead(upSwitch);
  boolean downRaw = digitalRead(downSwitch);

  // -------------------------
  // UPスイッチのデバウンス
  // -------------------------
  if (upRaw == upLastRaw) {
    if (upDebounceCount < DEBOUNCE_COUNT) {
      upDebounceCount++;
    }
  } else {
    upDebounceCount = 0;
    upLastRaw = upRaw;
  }

  if (upDebounceCount >= DEBOUNCE_COUNT) {
    upStable = upRaw;
  }

  // -------------------------
  // DOWNスイッチのデバウンス
  // -------------------------
  if (downRaw == downLastRaw) {
    if (downDebounceCount < DEBOUNCE_COUNT) {
      downDebounceCount++;
    }
  } else {
    downDebounceCount = 0;
    downLastRaw = downRaw;
  }

  if (downDebounceCount >= DEBOUNCE_COUNT) {
    downStable = downRaw;
  }

  // 押された瞬間だけ検出
  boolean upPressedEdge   = (upPrevStable == HIGH && upStable == LOW);
  boolean downPressedEdge = (downPrevStable == HIGH && downStable == LOW);

  upPrevStable   = upStable;
  downPrevStable = downStable;

  boolean upRepeat   = false;
  boolean downRepeat = false;

  // -------------------------
  // UP長押しリピート
  // -------------------------
  if (upStable == LOW && downStable == HIGH) {
    if (upHoldCount < 255) {
      upHoldCount++;
    }

    if (upHoldCount >= REPEAT_START_COUNT) {
      if (upRepeatCount == 0) {
        upRepeat = true;
        upRepeatCount = REPEAT_NEXT_COUNT;
      } else {
        upRepeatCount--;
      }
    }
  } else {
    upHoldCount = 0;
    upRepeatCount = 0;
  }

  // -------------------------
  // DOWN長押しリピート
  // -------------------------
  if (downStable == LOW && upStable == HIGH) {
    if (downHoldCount < 255) {
      downHoldCount++;
    }

    if (downHoldCount >= REPEAT_START_COUNT) {
      if (downRepeatCount == 0) {
        downRepeat = true;
        downRepeatCount = REPEAT_NEXT_COUNT;
      } else {
        downRepeatCount--;
      }
    }
  } else {
    downHoldCount = 0;
    downRepeatCount = 0;
  }

  boolean volumeChanged = false;

  // -------------------------
  // UP処理
  // 押した瞬間 or 長押しリピート
  // -------------------------
  if ((upPressedEdge || upRepeat) && downStable == HIGH) {
    if (volumeCounter > 0) {
      volumeCounter--;
      volumeChanged = true;
    }
  }

  // -------------------------
  // DOWN処理
  // 押した瞬間 or 長押しリピート
  // -------------------------
  else if ((downPressedEdge || downRepeat) && upStable == HIGH) {
    if (volumeCounter < 255) {
      volumeCounter++;
      volumeChanged = true;
    }
  }

  // 変更があった時だけDACのATTレジスタに書き込む
  if (volumeChanged) {
    uint8_t i;
    for (i = 0; i <= ptrSlave; i++) {
      i2cWrite(BD34301_CHIP[i], Volume1, volumeCounter);
      i2cWrite(BD34301_CHIP[i], Volume2, volumeCounter);
    }
  }
}
