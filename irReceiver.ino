uint8_t irReceiver() {

  controlByIR();
  return(count);
}

void translateIR() // takes action based on IR code received

// describing Remote IR codes 

{

  switch(results.value)

  {
  case 0xFFA25D: Serial.println("POWER"); break;
  case 0xFFE21D: Serial.println("FUNC/STOP"); break;
  case 0xFF629D: Serial.println("VOL+"); break;
  case 0xFF22DD: Serial.println("FAST BACK");    break;
  case 0xFF02FD: Serial.println("PAUSE");    break;
  case 0xFFC23D: Serial.println("FAST FORWARD");   break;
  case 0xFFE01F: Serial.println("DOWN");    break;
  case 0xFFA857: Serial.println("VOL-");    break;
  case 0xFF906F: Serial.println("UP");    break;
  case 0xFF9867: Serial.println("EQ");    break;
  case 0xFFB04F: Serial.println("ST/REPT");    break;
  case 0xFF6897: Serial.println("0");    break;
  case 0xFF30CF: Serial.println("1");    break;
  case 0xFF18E7: Serial.println("2");    break;
  case 0xFF7A85: Serial.println("3");    break;
  case 0xFF10EF: Serial.println("4");    break;
  case 0xFF38C7: Serial.println("5");    break;
  case 0xFF5AA5: Serial.println("6");    break;
  case 0xFF42BD: Serial.println("7");    break;
  case 0xFF4AB5: Serial.println("8");    break;
  case 0xFF52AD: Serial.println("9");    break;
  /* Apple Remote */
  case 0x77E1504F: Serial.println("APPLE UP"); break;
  case 0x77E1304F: Serial.println("APPLE DOWN");  break;
  case 0x77E1904F: Serial.println("APPLE LEFT");  break;
  case 0x77E1604F: Serial.println("APPLE RHIGT"); break;
  case 0x77E1C04F: Serial.println("APPLE MENU");  break;
  case 0x77E1FA4F: Serial.println("APPLE PLAY/PAUSE");  break;
  case 0x77E13A4F: Serial.println("APPLE OK");  break;  // 0x77E1A04F
  /* OptoSupply */
  case 0x8F705FA: Serial.println("OptoSupply ^"); break;
  case 0x8F704FB: Serial.println("OptoSupply o"); break;
  case 0x8F700FF: Serial.println("OptoSupply v"); break;
  case 0x8F708F7: Serial.println("OptoSupply <-"); break;
  case 0x8F701FE: Serial.println("OptoSupply ->"); break;
  case 0x8F78D72: Serial.println("OptoSupply ^¥"); break;
  case 0x8F7847B: Serial.println("OptoSupply /^"); break;
  case 0x8F78877: Serial.println("OptoSupply v/"); break;
  case 0x8F7817E: Serial.println("OptoSupply ¥v"); break;
  case 0x8F71FE0: Serial.println("OptoSupply A"); break;
  case 0x8F71EE1: Serial.println("OptoSupply B"); break;
  case 0x8F71AE5: Serial.println("OptoSupply C"); break;
  case 0x8F71BE4: Serial.println("OptoSupply POWER"); break;
  
  case 0xFFFFFFFF: Serial.println(" REPEAT");break;

  default: 
    Serial.println(" other button   ");

  }

//  delay(100); // Do not get immediate repeat

}

void controlByIR()
{
  if (irrecv.decode(&results)) // have we received an IR signal?

  {
    // rtranslateIR();
    // リモコンのUPが押された場合
    if ( (results.value == 0x77E1504F) || (results.value == 0x8F705FA) ) {
    // volumeCounterの値が0だったら
      if ( volumeCounter == 0 ){
        // volumeCounterの値を保持する
        volumeCounter = volumeCounter;
      }
      // volumeCounterの値が0以外であれば
      else {
        // volumeCounterの値をデクリメントする
        volumeCounter--;
      }
    }
    // リモコンのDOWNが押された場合
    else if ( (results.value == 0x77E1304F) || (results.value == 0x8F700FF) ) {
    // volumeCounterが255になったら
      if ( volumeCounter == 255 ) {
        // volumeCounterの値を保持する
        volumeCounter = volumeCounter;
      }
      // volumeCounterの値が255以外であれば
      else {
        // volumeCounterをインクリメントする
        volumeCounter++;
      }    
    }
    // リモコンのLEFTが押されたら
    else if ( (results.value == 0x77E1904F) || (results.value == 0x8F708F7) ) {
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
    // DACのATTレジスタにvolumeCounterの値を設定する
    uint8_t i;
    for(i=0; i<=ptrSlave; i++){
      i2cWrite(BD34301_CHIP[i], 0x21, volumeCounter);
      i2cWrite(BD34301_CHIP[i], 0x22, volumeCounter);
    }
    irrecv.resume(); // receive the next value
  }
}
