/*************************************************************
  入力ソースの選択
  **************
  
  タクトスイッチが押されることによって入力インターフェースを切り替える．
  タイマー割り込みで呼ばれ、スイッチが押される毎に
  
    USB -> RJ45 -> XH
  
  と切り替わる.スイッチの切り替えはスイッチがOFFの状態からONの状態に変化が
  あった場合に実行される．
  
 *************************************************************/
uint8_t inputSelection() {

  bool state = digitalRead(inputSwitch);
  static bool inswState = HIGH;


  // スイッチが押されたことの変化があった場合
  if ( inswState == HIGH && state == LOW) {
    // countに1を足して
    count++;
    if ( (HWCNF[10] == 0x00) ) {
      // countが1の場合
      if ( count == 1 ) { // USB
        // 入力をUSBにする
        i2cWrite(CPLD_ADR, 0x00, 0x00);
        // digitalWrite(INSEL0, LOW);
        // digitalWrite(INSEL1, LOW);
      }
      // countが2の場合
      else if (count == 2) {  // XH
        // 入力をXHコネクタ(I2S)にする
        i2cWrite(CPLD_ADR, 0x00, 0x10);
        // digitalWrite(INSEL0, LOW);
        // digitalWrite(INSEL1, HIGH);
        count = 0;
      }
    }
    else if ( HWCNF[10] == 0x40) {
      if ( count == 1) {  // USB
        // 入力をUSBにする
        i2cWrite(CPLD_ADR, 0x00, 0x00);
        // digitalWrite(INSEL0, LOW);
        // digitalWrite(INSEL1, LOW);        
      }
      else if ( count == 2 ) {
        // 入力をRJ45コネクタ（LANケーブル経由のI2S)にする
        i2cWrite(CPLD_ADR, 0x00, 0x08);
        // digitalWrite(INSEL0, HIGH);
        // digitalWrite(INSEL1, LOW);        
      } 
      else if ( count == 3 ) {  // 入力をXHコネクタ(I2S)にする
        i2cWrite(CPLD_ADR, 0x00, 0x10);
        // digitalWrite(INSEL0, LOW);
        // digitalWrite(INSEL1, HIGH);        
        count = 0;
      }
    }

    else if (HWCNF[10] == 0xC0) {
      //Serial.println("MULTI OPTION");
      // countが1の場合
      if ( count == 1 ) {
        // 入力をUSBにする
        i2cWrite(CPLD_ADR, 0x00, 0x00);
        // digitalWrite(INSEL0, LOW);
        // digitalWrite(INSEL1, LOW);
        // i2cWrite(CPLD_ADR, 0x00, 0x00); // USB
      }
      else if (count == 2) {
        // 入力をRJ45コネクタ（LANケーブル経由のI2S)にする
        i2cWrite(PCM9211_ADR, 0x7C, 0x01);  // LVC541出力を無効化
        i2cWrite(PCM9211_ADR, 0x78, 0x22);  // LVC157出力を有効化、LVDSを選択
        // オプションコネクタを選択
        i2cWrite(CPLD_ADR, 0x00, 0x08);
        // digitalWrite(INSEL0, HIGH);
        // digitalWrite(INSEL1, LOW);
      }
      // countが3の場合
      else if (count == 3) {
        // 入力をXHコネクタ(I2S)にする
        i2cWrite(CPLD_ADR, 0x00, 0x10);
        // digitalWrite(INSEL0, LOW);
        // digitalWrite(INSEL1, HIGH);  
        // シリアルモニタに出力
        //Serial.println("XH INPUT Selected");
      }
      else if ( count == 4 ) {
        // 入力をOpticalにする
        i2cWrite(PCM9211_ADR, 0x7c, 0x01);  // LVC541出力を無効化
        i2cWrite(PCM9211_ADR, 0x34, 0xC4);  // Optical入力を選択
        i2cWrite(PCM9211_ADR, 0x78, 0x21);  // LVC157出力を有効化,PCM9211出力を選択
        // オプションコネクタを選択
        i2cWrite(CPLD_ADR, 0x00, 0x08);
        // digitalWrite(INSEL0, HIGH);
        // digitalWrite(INSEL1, LOW);
      }
      else if ( count == 5 ) {
        // 入力をCoaxialにする
        i2cWrite(PCM9211_ADR, 0x34, 0x40);  // Coaxial入力を選択
      }
      else if ( count == 6 ) {
        // 入力をマルチオプション基板のXHコネクタ(I2S)にする
        i2cWrite(PCM9211_ADR, 0x78, 0x12);  // LVC157出力を無効化
        i2cWrite(PCM9211_ADR, 0x7c, 0x00);  // LVC541出力を有効化
        count = 0;
      }
    }
    //Serial.print("count = "); Serial.println(count);
  }
  inswState = state;
  return(count);
}