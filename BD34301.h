#define CPLD_ADR 0x52
#define upSwitch 4
#define downSwitch 39
#define filterSwitch 35
#define inputSwitch 34
#define pwLED 25
#define DP 5

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

int receiver = 32; // Signal Pin of IR receiver to Arduino Digital Pin 32

Preferences preferences;
int volumeValue; 

int volumeCounter;
//int state;
volatile int cnt = 3;
volatile int count = 1;
//volatile int buttonState = HIGH;
//volatile int inswState = HIGH;
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

char freq32[]           = "  32kHz  ";
char freq44[]           = "  44.1kHz";
char freq48[]           = "  48kHz  ";
char freq88[]           = "  88.2kHz";
char freq96[]           = "  96kHz  ";
char freq176[]          = " 176.4kHz";
char freq192[]          = " 192kHz  ";
char freq352[]          = " 352.8kHz";
char freq384[]          = " 384kHz  ";
char freqBlank[]        = "         ";
char freqDsd64[]        = " 2.8MHz  ";
char freqDsd128[]       = " 5.6MHz  ";
char freqDsd256[]       = " 11.2MHz ";
char freqDsd512[]       = " 22.4MHz ";

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
char bd34301[]          = "BD34301EKV";
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

volatile int DSDON;
//volatile int FS;
//volatile int DSD64;
//volatile int mono;
uint8_t dsdOn, pcmRate, dsdRate;
uint8_t digiFil = 1;
uint8_t inputSource = 1;
//int deviceName;
//uint8_t DEVNAME,INSEL,DIF,MONO_ST;
//int DEM,DSDF;
//bool DSDD;
//bool GC0,GC1;
//bool mute = true;

//int prevMode = 1;
//int prevPcmRate = 0;
//int prevDsdRate = 0;
//int prevFil = 1;
//uint8_t prevInputSource = 1;

uint8_t HWCNF[10]; //{DEVNAME, INSEL, DIF, MONO_ST, DSDF, INPOL, DEM, OSR, HPC, PAC};
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
