/*************************************************************
  入力ソースの選択
  **************
  
  タクトスイッチが押されることによって入力インターフェースを切り替える．
  タイマー割り込みで呼ばれ、スイッチが押される毎に
  
    USB -> RJ45 -> XH
  
  と切り替わる.スイッチの切り替えはスイッチがOFFの状態からONの状態に変化が
  あった場合に実行される．
  
  電源オン時、ESP32のリセット時はUSBとなる.
 *************************************************************/
uint8_t inputSelection() {

  bool state = digitalRead(inputSwitch);
  static bool inswState = HIGH;
  // スイッチが押されたことの変化があった場合
  if ( inswState == HIGH && state == LOW) {
    // countに1を足して
    count++;
    // countが1の場合
    if ( count == 1 ) {
      // 入力をUSBにする
      i2cWrite(CPLD_ADR, 0x00, 0x00); // USB
      //i2cWrite(CPLD_ADR, 0x00, 0x10); // XH
      //i2cWrite(CPLD_ADR, 0x00, 0x08); // RJ45
      // シリアルモニタに出力
      //Serial.println("USB INPUT Selected");
    }
    // countが2の場合
    else if (count == 2) {
      // 入力をRJ45コネクタ（LANケーブル経由のI2S)にする
      i2cWrite(CPLD_ADR, 0x00, 0x08); // RJ45
      //i2cWrite(CPLD_ADR, 0x00, 0x00); // USB
      // シリアルモニタに出力
      //Serial.println("RJ45 INPUT Seleted");
    }
    // countが3の場合
    else if (count == 3) {
      // 入力をXHコネクタ(I2S)にする
      i2cWrite(CPLD_ADR, 0x00, 0x10);  // XH
      //i2cWrite(CPLD_ADR, 0x00, 0x08);  // RJ45
      // countを0にする
      count = 0;
      // シリアルモニタに出力
      //Serial.println("XH INPUT Selected");
    }
  }
  inswState = state;
  return(count);
}
