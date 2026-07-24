#include <Wire.h>
#include "SO2002A_I2C.h"
#include <U8g2lib.h>
#include <Preferences.h>
#include "driver/pcnt.h"
#include "BD34301.h"
#include "IRremote.h"
#include <WiFi.h>
#include "esp_bt.h"

#include <IRremote.hpp>

#define SDA 21
#define SCL 22

SO2002A_I2C oled(0x3D);
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SCL, SDA, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  pinMode(upSwitch,INPUT);
  pinMode(downSwitch,INPUT);
  pinMode(filterSwitch,INPUT);
  pinMode(inputSwitch,INPUT);
  pinMode(pwLED, OUTPUT);
  pinMode(DP, INPUT);
  pinMode(APPLE_PAIR_RESET_PIN, INPUT_PULLUP);
  pinMode(INSEL0, OUTPUT);
  pinMode(INSEL1, OUTPUT);
  pinMode(AUDIO_BCLK_PIN, INPUT);
  pinMode(AUDIO_LRCK_PIN, INPUT);

  
  // Setup timer interrupt
  // Timer: interrupt time and event setting. 
  timer1 = timerBegin(0, 80, true);
  //timer2 = timerBegin(1, 80, true);
  timer3 = timerBegin(2, 80, true);
  timer4 = timerBegin(3, 80, true);
    
  // Attach onTimer function.
  timerAttachInterrupt(timer1, &onTimer1, true);
  //timerAttachInterrupt(timer2, &onTimer2, true);
  timerAttachInterrupt(timer3, &onTimer3, true);
  timerAttachInterrupt(timer4, &onTimer4, true);
  
  // Set alarm to call onTimer function every second (value in microseconds).
  timerAlarmWrite(timer1, 10000, true); // 10ms
  //timerAlarmWrite(timer2, 150000, true); // 150ms
  timerAlarmWrite(timer3, 200000, true); // 200ms
  timerAlarmWrite(timer4, 200000, true); // 200ms
  
  // Start an alarm
  timerAlarmEnable(timer1);
  //timerAlarmEnable(timer2);
  timerAlarmEnable(timer3);
  timerAlarmEnable(timer4);

  // NVRAM setting
  volPrefs.begin("volume", false);
  volumeValue = volPrefs.getInt("value", 0);
  volPrefs.end();

  if (volumeCounter != volumeValue ) {
    volPrefs.putInt("value", volumeCounter);
    volPrefs.end();
  }

  volumeCounter = volumeValue;
  //
  Serial.begin(115200);
  delay(500);

  WiFi.mode(WIFI_OFF);
  btStop();

  Wire.begin(SDA, SCL);
  //Wire.setTimeOut(100);
  // SCLの周波数を400kHzに設定する
  Wire.setClock(400000);
 
  initAudioSignalDetector();
  
  oled.begin(20, 2);
  oled.clear();
 
  delay(500);
 
  i2cWrite(CPLD_ADR, 0x03, 0x01); // MCLKの停止を解除
  i2cWrite(CPLD_ADR, 0x03, 0x81); // RESETB解除　
  
  delay(10);

  /* コンフィグピンのステータスを取得 */
  getInitialSetting(); 

  /* 20x2 OLED表示器の初期化 */
  initSO2002A();

  // デバッグ用のLEDを点灯
  digitalWrite(pwLED,HIGH);

  
  // irrecv.enableIRIn(); // Start the receiver

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  irPrefs.begin("apple_ir", false);

  applePaired = irPrefs.getBool("paired", false);
  pairedAppleAddress = irPrefs.getUShort("addr", 0x0000);

  if (applePaired) {
    Serial.print("Apple Remote paired address loaded: 0x");
    Serial.println(pairedAppleAddress, HEX);
  }
  else {
    Serial.println("Apple Remote is not paired.");
  }

  // 入力ソースの初期選択
  // 常にUSBを優先
  if ((HWCNF[10] == 0x00)) {  // USB ans XH
    i2cWrite(CPLD_ADR, 0x00, 0x00);
    // digitalWrite(INSEL0, LOW);
    // digitalWrite(INSEL1, LOW);
    }
  else if (HWCNF[10] == 0x40) { // USB,XH and RJ45
    i2cWrite(CPLD_ADR, 0x00, 0x00);
    // digitalWrite(INSEL0, LOW);
    // digitalWrite(INSEL1, LOW);
  }
  else if (HWCNF[10] == 0xC0) {
    i2cWrite(CPLD_ADR, 0x00, 0x00);
    // digitalWrite(INSEL0, LOW);
    // digitalWrite(INSEL1, LOW);
  }

  /* 電源立ち上げシーケンス */
  bootUp();
  readReg(0);
}

void loop() {
  irReceiver();
  // Timer interrupt process
  if (timeCounter1 > 0) {
    portENTER_CRITICAL(&timerMux);
    timeCounter1--;
    portEXIT_CRITICAL(&timerMux);
    volumeControl();
  }

  if (timeCounter3 > 0) {
    portENTER_CRITICAL(&timerMux);
    timeCounter3--;
    portEXIT_CRITICAL(&timerMux);
    changeFilter();
  }
  
  if (timeCounter4 > 0) {
    portENTER_CRITICAL(&timerMux);
    timeCounter4--;
    portEXIT_CRITICAL(&timerMux);
    inputSelection();
  }
  
  volPrefs.begin("volume", false);
  if (volumeCounter != volumeValue ) {
    volPrefs.putInt("value", volumeCounter);
    volPrefs.end();
  }

uint16_t FSR = detectFS();

#if AUDIO_DEBUG
  static uint32_t lastAudioDebugTime = 0;

  if ((millis() - lastAudioDebugTime) >= 500) {
    lastAudioDebugTime = millis();

    Serial.print("BCLK count=");
    Serial.print(measuredBclkCount);

    Serial.print(", LRCK/DATA count=");
    Serial.print(measuredLrckCount);

    Serial.print(", BCLK=");
    Serial.print(measuredBclkHz);

    Serial.print(" Hz, LRCK/DATA edge=");
    Serial.print(measuredLrckEdgeHz);

    Serial.print(" Hz, ratio=");

    if (measuredBclkHz != 0) {
      Serial.print(
        (float)measuredLrckEdgeHz /
        (float)measuredBclkHz,
        4);
    }
    else {
      Serial.print("0.0000");
    }

    Serial.print(", candidate=");

    switch (candidateAudioMode) {
      case AUDIO_MODE_PCM:
        Serial.print("PCM");
        break;

      case AUDIO_MODE_DSD:
        Serial.print("DSD");
        break;

      default:
        Serial.print("NONE");
        break;
    }

    Serial.print(", candidateFS=");
    Serial.print(candidateAudioFS);

    Serial.print(", confirmed=");

    switch (detectedAudioMode) {
      case AUDIO_MODE_PCM:
        Serial.print("PCM");
        break;

      case AUDIO_MODE_DSD:
        Serial.print("DSD");
        break;

      default:
        Serial.print("NONE");
        break;
    }

    Serial.print(", dsdOn=");
    Serial.print(dsdOn);

    Serial.print(", FSR=");
    Serial.println(FSR);
  }
#endif

modeSwitch(FSR, digiFil, count);
messageOut(FSR, digiFil);

  delay(10);
}

uint8_t i2cRead(uint8_t sladr, uint8_t regadr){
  Wire.beginTransmission(sladr);
  Wire.write(regadr);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)sladr, (uint8_t)1);
  return Wire.read();
}

uint8_t i2cWrite(uint8_t sladr, uint8_t regadr, uint8_t wdata){
  Wire.beginTransmission(sladr);
  Wire.write(regadr);
  Wire.write(wdata);
  return Wire.endTransmission();
}

uint8_t detectBitClock() {
  cpld.sampleRate = i2cRead(CPLD_ADR, 0x03);
  uint8_t bck16 = cpld.sampleRate & 0x80;
  return(bck16);
}

void getInitialSetting() {
  uint8_t temp = i2cRead(CPLD_ADR, 0x00);
  HWCNF[0] = temp & 0x07; // Device Name
  HWCNF[1] = temp & 0x18; // Input Select
  HWCNF[10] = temp & 0xC0;  // Detect Option Board
  int hwcnf = HWCNF[10];
  Serial.print("HWCNF[10] = "); Serial.println(hwcnf);
  temp = i2cRead(CPLD_ADR, 0x01);
  HWCNF[2] = temp & 0x38; // Digital Input Format
  HWCNF[3] = temp & 0xC0; // Stereo/Mono Mode
  HWCNF[5] = temp & 0x03; // Input Polarity
  HWCNF[8] = temp & 0x04; // HPC(High Precision Caluclation)
  temp = i2cRead(CPLD_ADR, 0x02);
  HWCNF[4] = temp & 0x03; // DSD Filter
  HWCNF[6] = temp & 0x10; // De-Emphasis
  HWCNF[7] = temp & 0x0C; // OSR(Over Sampling Rate)
  HWCNF[9] = temp & 0x20; // PAC(Phase Adjustment Control)
  
  HWCNF[11] = readChipVersion();  // Chip Version

  ptrSlave = ~(HWCNF[3] >> 6) & 0x03;
  Serial.print("ptrSlave="); Serial.println(ptrSlave);
}

void readReg(uint8_t chipNum) {
  uint8_t registerAddress[12] = {0x04, 0x06, 0x10, 0x16, 0x20, 0x30, 0x31, 0x40, 0x42, 0x43, 0x60, 0x61};
  uint8_t value, i;
  for (i=0; i<12; i++) {
    value = i2cRead(BD34301_CHIP[chipNum], registerAddress[i]);
    if (i==0) Serial.print("Clock 1 = ");
    else if (i==1) Serial.print("Clock 2 = ");
    else if (i==2) Serial.print("Audio IF1 = ");
    else if (i==3) Serial.print("DSD Filter = ");
    else if (i==4) Serial.print("Volume Transition Time = ");
    else if (i==5) Serial.print("FIR Filter1 = ");
    else if (i==6) Serial.print("FIR Filter2 = ");
    else if (i==7) Serial.print("Delta Sigma = ");
    else if (i==8) Serial.print("Setting 2 = ");
    else if (i==9) Serial.print("Setting 3 = ");
    else if (i==10) Serial.print("Setting 5 = ");
    else if (i==11) Serial.print("Setting 6 = ");
    Serial.print("0x");
    if (value<16) Serial.print("0");
    Serial.println(value, HEX);
  }
}

uint8_t readChipVersion() {
  uint8_t Version = i2cRead(BD34301_CHIP[0], ChipVersion);
  Serial.print("Chip Version = ");
  Serial.print("0x");
  if (Version<16) Serial.print("0");
  Serial.println(Version, HEX);
  return (Version);
}
