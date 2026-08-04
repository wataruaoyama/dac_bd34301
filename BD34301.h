#define CPLD_ADR 0x52
#define PCM9211_ADR 0x40
#define upSwitch 4
#define downSwitch 39
#define filterSwitch 35
#define inputSwitch 34
#define pwLED 25
#define DP 5

// -----------------------------------------------------------------------------
// CPLD write register 3
//
// 注意：CPLDのreg3はread側とwrite側が別レジスタ。
// read-modify-writeは使用せず、ESP32側のシャドーレジスタで管理する。
// -----------------------------------------------------------------------------
#define CPLD_REG_CONTROL       0x03

#define CPLD_REG3_MCLK_RUN     0x01  // D0
#define CPLD_REG3_MUTE_REQ     0x02  // D1: 1=MUTE
#define CPLD_REG3_DSD_MODE     0x04  // D2: 1=DSD
#define CPLD_REG3_RESETB       0x80  // D7

#define CPLD_MUTE_RELEASE_DELAY_MS 100

#define AUDIO_DEBUG 0
#define AUDIO_DEBUG_NOIZE 1

// -----------------------------------------------------------------------------
// Audio signal detector
// -----------------------------------------------------------------------------

// ESP32に接続する音声信号
#define AUDIO_BCLK_PIN 19
#define AUDIO_LRCK_PIN 18

// PCNTユニット
#define BCLK_PCNT_UNIT PCNT_UNIT_0
#define LRCK_PCNT_UNIT PCNT_UNIT_1

// 500us測定なら、49.152MHzでも24576カウントでPCNT上限内
#define AUDIO_MEASUREMENT_US 500

// 500usのFs測定中、50usごとにBCLK停止を確認する
#define AUDIO_FAST_CHECK_US 50

// 50us間のBCLKエッジがこれ未満なら停止と判断
#define AUDIO_FAST_MIN_BCLK_COUNT 10

// PCM/DSD判定閾値
#define DSD_EDGE_RATIO_THRESHOLD 0.080f

// 同じ判定がこの回数続いたら確定
#define AUDIO_DETECT_STABLE_COUNT 3

// 信号停止時に直前のレートを保持
#define AUDIO_HOLD_LAST_RATE 0

// BD343xx レジスタアドレス
#define SoftwareReset 0x00
#define ChipVersion 0x01
#define DigitalPower 0x02
#define AnalogPower 0x03
#define Clock1 0x04
#define Clock2 0x06
#define AudioIF1 0x10
#define AudioIF2 0x12
#define AudioIF3 0x13
#define AudioOutputPolarity 0x14
#define DSDFilter 0x16
#define AudioInputPolarity 0x17
#define VolumeTransitionTime 0x20
#define Volume1 0x21
#define Volume2 0x22
#define MuteTransitionTime 0x29
#define Mute 0x2A
#define RAMClear 0x2F
#define FIRFilter1 0x30
#define FIRFilter2 0x31
#define DeEmphasis1 0x33
#define DeEmphasis2 0x34
#define DeltaSigma 0x40
#define Setting1 0x41
#define Setting2 0x42
#define Setting3 0x43
#define Setting4 0x48
#define Setting5 0x60
#define Setting6 0x61
#define Boot1 0xD0
#define Boot2 0xD3

#define IR_RECEIVE_PIN 32

// IR長押し/連続操作調整
#define IR_HOLD_START_MS    400   // 長押しと判定するまでの時間
#define IR_REPEAT_STEP_MS   150   // 長押し中の連続変化間隔

static uint32_t irHoldStartTime = 0;
static uint32_t irLastStepTime = 0;
static uint8_t  irHoldCommand = 0;

// Apple Remote A1294 実機確認済み command
#define APPLE_MENU    0x03
#define APPLE_PLAY    0x5F
#define APPLE_RIGHT   0x06
#define APPLE_LEFT    0x09
#define APPLE_UP      0x0A
#define APPLE_DOWN    0x0C
#define APPLE_CENTER  0x5C

// OptoSupply NEC address
#define OPTO_ADDR     0x10

// OptoSupply command
#define OPTO_UP       0xA0
#define OPTO_CENTER   0x20
#define OPTO_DOWN     0x00
#define OPTO_LEFT     0x10
#define OPTO_RIGHT    0x80
#define OPTO_A        0xF8
#define OPTO_B        0x78
#define OPTO_C        0x58
#define OPTO_POWER    0xD8
// old raw       new address/command
// 0x08F705FA -> Address=0x10 Command=0xA0  // UP
// 0x08F704FB -> Address=0x10 Command=0x20  // CENTER
// 0x08F700FF -> Address=0x10 Command=0x00  // DOWN
// 0x08F708F7 -> Address=0x10 Command=0x10  // LEFT
// 0x08F701FE -> Address=0x10 Command=0x80  // RIGHT
// 0x08F71FE0 -> Address=0x10 Command=0xF8  // A
// 0x08F71EE1 -> Address=0x10 Command=0x78  // B
// 0x08F71AE5 -> Address=0x10 Command=0x58  // C
// 0x08F71BE4 -> Address=0x10 Command=0xD8  // POWER

// 秋月などの小型リモコン
#define NEC_REPEAT    0xFFFFFFFFUL

// Apple Remote ペアリング保存用
Preferences irPrefs;

static bool applePaired = false;
static uint16_t pairedAppleAddress = 0x0000;

// Apple Remote repeat処理用
static uint8_t lastAppleCommand = 0;

// OptoSupply / NEC repeat処理用
static uint32_t lastNecValue = 0;

// Apple Remote CENTER/OK 長押し解除用
#define APPLE_CENTER_HOLD_TIME_MS  2000

static bool appleCenterHolding = false;
static bool appleCenterUnpairDone = false;
static uint32_t appleCenterStartTime = 0;

// Apple Remote ペアリング強制解除ピン
// GPIO12をLowにすると解除
#define APPLE_PAIR_RESET_PIN  27

// チャタリング対策
#define PAIR_RESET_DEBOUNCE_MS  50

// int receiver = 32; // Signal Pin of IR receiver to Arduino Digital Pin 32

// Volume レベル保存用
Preferences volPrefs; //preferences;
int volumeValue; 

int volumeCounter;
volatile int cnt = 3;
volatile int count = 1;
volatile int blynkModeButton;
volatile int blynkMuteButton;

uint8_t BD34301_CHIP[4]={0x1C, 0x1D, 0x1E, 0x1F};

char Sharp[]            = "Sharp            ";
char Slow[]             = "Slow             ";
char ShortDelaySharp[]  = "Short Delay Sharp";
char ShortDelayShrp[]   = "Short Delay Shrp ";
char ShortDelaySlow[]   = "Short Delay Slow ";
char SuperSlow[]        = "Super Slow       ";
char LowDispersion[]    = "Low Dispersion   ";
char filterBlank[]      = "                 ";

char noSignal[]         = "     NO SIGNAL      ";
char freq32[]           = "  32kHz   ";
char freq44[]           = "  44.1kHz ";
char freq48[]           = "  48kHz   ";
char freq88[]           = "  88.2kHz ";
char freq96[]           = "  96kHz   ";
char freq176[]          = " 176.4kHz ";
char freq192[]          = " 192kHz   ";
char freq352[]          = " 352.8kHz ";
char freq384[]          = " 384kHz   ";
char freqBlank[]        = "          ";
char freqDsd64[]        = " 2.8MHz   ";
char freqDsd128[]       = " 5.6MHz   ";
char freqDsd256[]       = " 11.2MHz  ";
char freqDsd512[]       = " 22.4MHz  ";

char audioIF0[]         = "16bit LSB";
char audioIF1[]         = "20bit LSB";
char audioIF2[]         = "24bit MSB";
char audioIF3[]         = "24bit I2S";
char audioIF4[]         = "24bit LSB";
char audioIF5[]         = "32bit LSB";
char audioIF6[]         = "32bit MSB";
char audioIF7[]         = "32bit I2S";
char audioIF8[]         = "16bit I2S";

char dsdNormal[]        = "Normal";
char dsdBypass[]        = "Bypass";

char demOff[]           = "OFF";
char demOn[]            = "ON ";

char ak4499[]           = "AK4499";
char ak4493[]           = "AK4493";
char ak4495[]           = "AK4495S";
char ak4490[]           = "AK4490";
char bd34301[]          = "BD34301";
char bd34352[]          = "BD34352";
char es9038q[]          = "ES9038Q2M";
char others[]           = "Others";

char currentOut0[]      = "72m/72m/45mA";
char currentOut1[]      = "72m/45m/45mA";
char currentOut2[]      = "45m/45m/45mA";
char currentOut3[]      = "45m/45m/45mA";
char outputLevel0[]     = "5.6/5.6/5Vpp";
char outputLevel1[]     = "5.6/5/5Vpp";
char outputLevel2[]     = "5/5/5Vpp";
char outputLevel3[]     = "5/5/5Vpp";

char DigitalMute[]      = "   MUTE";

volatile int DSDON;

uint8_t dsdOn, pcmRate, dsdRate;
uint8_t digiFil = 1;
uint8_t inputSource = 1;

enum AudioSignalMode {
  AUDIO_MODE_NONE = 0,
  AUDIO_MODE_PCM,
  AUDIO_MODE_DSD
};

// 確定済み判定
AudioSignalMode detectedAudioMode = AUDIO_MODE_NONE;

// 安定化判定用
AudioSignalMode pendingAudioMode  = AUDIO_MODE_NONE;

// 今回1回分の測定結果
AudioSignalMode candidateAudioMode = AUDIO_MODE_NONE;
uint16_t candidateAudioFS = 0;

// 確定・安定用
uint16_t lastValidFS = 0;
uint16_t pendingFS = 0;
uint8_t audioStableCount = 0;

uint32_t measuredBclkHz = 0;
uint32_t measuredLrckEdgeHz = 0;

int16_t measuredBclkCount = 0;
int16_t measuredLrckCount = 0;

bool displayMute = false;

bool cpldEarlyMuteActive = false;

uint8_t HWCNF[12]; //{DEVNAME, INSEL, DIF, MONO_ST, DSDF, INPOL, DEM, OSR, HPC, PAC, OPT, CHIP_VERSION};
uint8_t ptrSlave;


// Timer 
volatile int timeCounter1;
//volatile int timeCounter2;
volatile int timeCounter3;
volatile int timeCounter4;
hw_timer_t *timer1 = NULL;
//hw_timer_t *timer2 = NULL;
hw_timer_t *timer3 = NULL;
hw_timer_t *timer4 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer1(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  timeCounter1++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

//void IRAM_ATTR onTimer2(){
//  // Increment the counter and set the time of ISR
//  portENTER_CRITICAL_ISR(&timerMux);
//  timeCounter2++;
//  portEXIT_CRITICAL_ISR(&timerMux);
//}

void IRAM_ATTR onTimer3(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  timeCounter3++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR onTimer4(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  timeCounter4++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

typedef struct registerMap2 {
  uint8_t hwConfig;
  uint8_t deviceConfig0;
  uint8_t deviceConfig1;
  uint8_t sampleRate;
} registerMap2;

registerMap2 cpld;
