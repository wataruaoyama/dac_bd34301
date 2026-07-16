#include <Wire.h>
#include "SO2002A_I2C.h"
#include <U8g2lib.h>
#include <Preferences.h>
#include "BD34301.h"
#include "IRremote.h"
#include <WiFi.h>
#include "esp_bt.h"

#include <IRremote.hpp>

#define SDA 21
#define SCL 22

SO2002A_I2C oled(0x3D);
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SCL, SDA, /* reset=*/ U8X8_PIN_NONE);

/*-----( Declare objects )-----*/
// IRrecv irrecv(receiver);     // create instance of 'irrecv'
// decode_results results;      // create instance of 'decode_results'

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
    digitalWrite(INSEL0, LOW);
    digitalWrite(INSEL1, LOW);
    }
  else if (HWCNF[10] == 0x40) { // USB,XH and RJ45
    digitalWrite(INSEL0, LOW);
    digitalWrite(INSEL1, LOW);
  }
  else if (HWCNF[10] == 0xC0) {
    digitalWrite(INSEL0, LOW);
    digitalWrite(INSEL1, LOW);
  }

  // if ((HWCNF[10] == 0x00)) i2cWrite(CPLD_ADR, 0x00, 0x00);  // USB ans XH
  // else if (HWCNF[10] == 0x40) i2cWrite(CPLD_ADR, 0x00, 0x00); // USB,XH and RJ45
  // else if (HWCNF[10] == 0x40) i2cWrite(CPLD_ADR, 0x00, 0x08); // RJ45
  // else if (HWCNF[10] == 0x60) i2cWrite(CPLD_ADR, 0x00, 0x00); // USB
  // else if (HWCNF[10] == 0xC0) i2cWrite(CPLD_ADR, 0x00, 0x00); // USB

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

  uint16_t FSR = detectFS(); //Serial.print("FSR = "); Serial.println(FSR);
  //uint8_t BCK16 = detectBitClock(); Serial.print("BCK16 = "); Serial.println(BCK16);
  //Serial.print("volumeCounter ="); Serial.println(volumeCounter);

  modeSwitch(FSR, digiFil, count);
  messageOut(FSR, digiFil);

  //readReg(0);
  
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

uint16_t detectFS() {
  static uint16_t lastFSR = 0;
  static uint8_t lastSampleRate = 0xFF;

  uint8_t sampleRate1;
  uint8_t sampleRate2;
  uint16_t FSR;

  // CPLDのサンプリングレートレジスタを2回読む
  sampleRate1 = i2cRead(CPLD_ADR, 0x03);
  delayMicroseconds(200);
  sampleRate2 = i2cRead(CPLD_ADR, 0x03);

  // 2回の読み取り値が一致しない場合は、前回値を保持する
  if (sampleRate1 != sampleRate2) {
    return lastFSR;
  }

  // ここから先は一致した値だけを使う
  cpld.sampleRate = sampleRate1;
  lastSampleRate = cpld.sampleRate;

  pcmRate = cpld.sampleRate & 0x3C;
  dsdRate = cpld.sampleRate & 0x42;
  dsdOn   = cpld.sampleRate & 0x01;

  if (dsdOn == 0x00) {
    if      (pcmRate == 0x00) FSR = 44;
    else if (pcmRate == 0x04) FSR = 32;
    else if (pcmRate == 0x08) FSR = 48;
    else if (pcmRate == 0x0C) FSR = 88;
    else if (pcmRate == 0x10) FSR = 96;
    else if (pcmRate == 0x14) FSR = 176;
    else if (pcmRate == 0x18) FSR = 192;
    else if (pcmRate == 0x1C) FSR = 352;
    else if (pcmRate == 0x20) FSR = 384;
    else                      FSR = lastFSR;  // 不正値なら前回保持
  } else {
    if      (dsdRate == 0x00) FSR = 2822;   // DSD64
    else if (dsdRate == 0x02) FSR = 5644;   // DSD128
    else if (dsdRate == 0x40) FSR = 11289;  // DSD256
    else if (dsdRate == 0x42) FSR = 22579;  // DSD512
    else                      FSR = lastFSR; // 不正値なら前回保持
  }

  lastFSR = FSR;
  return FSR;
}
// uint16_t detectFS() {
//   uint16_t FSR;
//   cpld.sampleRate = i2cRead(CPLD_ADR, 0x03);
//   pcmRate = cpld.sampleRate & 0x3C;
//   dsdRate = cpld.sampleRate & 0x42;
//   dsdOn = cpld.sampleRate & 0x01;
//   if (dsdOn == 0x00) {
//     if (pcmRate == 0x00 ) FSR = 44;
//     else if (pcmRate == 0x04) FSR = 32;
//     else if (pcmRate == 0x08) FSR = 48;
//     else if (pcmRate == 0x0C) FSR = 88;
//     else if (pcmRate == 0x10) FSR = 96;
//     else if (pcmRate == 0x14) FSR = 176;
//     else if (pcmRate == 0x18) FSR = 192;
//     else if (pcmRate == 0x1C) FSR = 352;
//     else if (pcmRate == 0x20) FSR = 384;
//     else FSR = 0;
//   }
//   else {
//     if (dsdRate == 0x00) FSR = 2822;
//     else if (dsdRate == 0x02) FSR = 5644;
//     else if (dsdRate == 0x40) FSR = 11289;
//     else if (dsdRate == 0x42) FSR = 22579;
//     else FSR = 0;
//   }
//   return(FSR);
// }

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
