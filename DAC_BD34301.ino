#include <Wire.h>
#include "SO2002A_I2C.h"
#include <U8g2lib.h>
#include <Preferences.h>
#include "BD34301.h"
#include "IRremote.h"

#define SDA 21
#define SCL 22

SO2002A_I2C oled(0x3D);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SCL, SDA, /* reset=*/ U8X8_PIN_NONE);

/*-----( Declare objects )-----*/
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

void setup() {
  pinMode(upSwitch,INPUT);
  pinMode(downSwitch,INPUT);
  pinMode(filterSwitch,INPUT);
  pinMode(inputSwitch,INPUT);
  pinMode(pwLED, OUTPUT);
  pinMode(DP, INPUT);

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
  timerAlarmWrite(timer1, 150000, true); // 150ms
  //timerAlarmWrite(timer2, 150000, true); // 150ms
  timerAlarmWrite(timer3, 200000, true); // 200ms
  timerAlarmWrite(timer4, 200000, true); // 200ms
  
  // Start an alarm
  timerAlarmEnable(timer1);
  //timerAlarmEnable(timer2);
  timerAlarmEnable(timer3);
  timerAlarmEnable(timer4);

  // NVRAM setting
  preferences.begin("my-app", false);
  volumeValue = preferences.getInt("value", 0);
  preferences.end();

  if (volumeCounter != volumeValue ) {
    preferences.putInt("value", volumeCounter);
    preferences.end();
  }

  volumeCounter = volumeValue;
  //
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  // SCLの周波数を400kHzに設定する
  Wire.setClock(400000);
  
  oled.begin(20, 2);
  oled.clear();
  
  delay(500);

  //i2cWrite(CPLD_ADR, 0x00, 0x10); // 入力ソースをXHに変更
  i2cWrite(CPLD_ADR, 0x03, 0x01); // MCLKの停止を解除
  i2cWrite(CPLD_ADR, 0x03, 0x81); // RESETB解除　

  /* コンフィグピンのステータスを取得 */
  getInitialSetting(); 
  /* 電源立ち上げシーケンス */
  bootUp();
  /* 20x2 OLED表示器の初期化 */
  initSO2002A();
  // デバッグ用のLEDを点灯
  digitalWrite(pwLED,HIGH);
  //readReg(0);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
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
  
  preferences.begin("my-app", false);
  if (volumeCounter != volumeValue ) {
    preferences.putInt("value", volumeCounter);
    preferences.end();
  }

  uint16_t FSR = detectFS();
  //Serial.print("FSR = "); Serial.println(FSR);
  uint8_t BCK16 = detectBitClock();
  //Serial.print("BCK16 = "); Serial.println(BCK16);
  //Serial.print("volumeCounter ="); Serial.println(volumeCounter);
  /*digiFil = */changeFilter();
  /*inputSource = */inputSelection();/* |*/ irReceiver();
  //Serial.print("Input Source = "); Serial.println(inputSource);
  modeSwitch(FSR, digiFil, inputSource);
  messageOut(FSR, digiFil);

  //readReg(0);
  
  delay(10);
}

uint8_t i2cRead(uint8_t sladr, uint8_t regadr){
  Wire.beginTransmission(sladr);
  Wire.write(regadr);
  Wire.endTransmission();
  Wire.requestFrom(sladr, 1);
  return Wire.read();
}

uint8_t i2cWrite(uint8_t sladr, uint8_t regadr, uint8_t wdata){
  Wire.beginTransmission(sladr);
  Wire.write(regadr);
  Wire.write(wdata);
  Wire.endTransmission();
}

uint16_t detectFS() {
  uint16_t FSR;
  cpld.sampleRate = i2cRead(CPLD_ADR, 0x03);
  pcmRate = cpld.sampleRate & 0x3C;
  dsdRate = cpld.sampleRate & 0x42;
  dsdOn = cpld.sampleRate & 0x01;
  if (dsdOn == 0x00) {
    if (pcmRate == 0x00 ) FSR = 32;
    else if (pcmRate == 0x04) FSR = 44;
    else if (pcmRate == 0x08) FSR = 48;
    else if (pcmRate == 0x0C) FSR = 88;
    else if (pcmRate == 0x10) FSR = 96;
    else if (pcmRate == 0x14) FSR = 176;
    else if (pcmRate == 0x18) FSR = 192;
    else if (pcmRate == 0x1C) FSR = 352;
    else if (pcmRate == 0x20) FSR = 384;
    else FSR = 0;
  }
  else {
    if (dsdRate == 0x00) FSR = 2822;
    else if (dsdRate == 0x02) FSR = 5644;
    else if (dsdRate == 0x40) FSR = 11289;
    else if (dsdRate == 0x42) FSR = 22579;
    else FSR = 0;
  }
  return(FSR);
}

uint8_t detectBitClock() {
  cpld.sampleRate = i2cRead(CPLD_ADR, 0x03);
  uint8_t bck16 = cpld.sampleRate & 0x80;
  return(bck16);
}

uint8_t getInitialSetting() {
  uint8_t temp = i2cRead(CPLD_ADR, 0x00);
  HWCNF[0] = temp & 0x07; // Device Name
  HWCNF[1] = temp & 0x18; // Input Select
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

  ptrSlave = ~(HWCNF[3] >> 6) & 0x03;
  Serial.print("ptrSlave="); Serial.println(ptrSlave);
}

void readReg(uint8_t chipNum) {
  uint8_t registerAddress[] = {0x04, 0x06, 0x10, 0x30, 0x31, 0x40, 0x60, 0x61};
  uint8_t value, i;
  for (i=0; i<8; i++) {
    value = i2cRead(BD34301_CHIP[chipNum], registerAddress[i]);
    if (i==0) Serial.print("Clock 1 = ");
    else if (i==1) Serial.print("Clock 2 = ");
    else if (i==2) Serial.print("Audio IF1 = ");
    else if (i==3) Serial.print("FIR Filter1 = ");
    else if (i==4) Serial.print("FIR Filter2 = ");
    else if (i==5) Serial.print("Delta Sigma = ");
    else if (i==6) Serial.print("Setting 5 = ");
    else if (i==7) Serial.print("Setting 6 = ");
    Serial.print("0x");
    if (value<16) Serial.print("0");
    Serial.println(value, HEX);
  }
}
