/*************************************************************
  メッセージ出力
  ************
  
  20x2 OLED表示装置にメッセージを出力する．
  
 *************************************************************/
void messageOut(uint16_t FS, uint8_t digiFil) {
  cpld.sampleRate = i2cRead(CPLD_ADR, 0x03);
  DSDON = cpld.sampleRate & 0x01;
//  FS = cpld.sampleRate & 0x3C;
//  DSD64 = cpld.sampleRate & 0x42;

  displayPlayMode();
  displayATT(volumeCounter);
  displayFs(FS);
  displayDigitalFilter(digiFil);
  displayInputInterface();
}

/* 再生モードの表示 */
void displayPlayMode() {
  // 再生モードがDSDの場合
  if ( DSDON == 0x01) {
    // (0.0)の位置にDSDを表示
    oled.setCursor(0, 0);
    oled.print("DSD");
  // PCMの場合
  } else if ( DSDON == 0x00 ){
    // (0.0)の位置にPCMを表示
    oled.setCursor(0, 0);
    oled.print("PCM");
  }
}

/* アッテネータレベルの表示 */
void displayATT(int vin) {
  int level;
  int decimals;

  // 0-255までの256ステップを0-127.5までの0.5ステップで表す為の小数点
  decimals = vin & 0x01;
  level = vin>>1;
  // 絶対値
  level = abs(level);
  // 表示位置を(13.0)にする
  oled.setCursor(13, 0);
  // データが10より小さい場合
  if ( (0 <= level) && (level <10)) {
    // 10で割った余りを一桁目の変数onesPlaceに代入
    int onesPlace = level % 10;
    // 2つのスペース
    oled.print("  ");
    // のあとにひと桁目の数字を表示
    oled.print(onesPlace);
    // decimalsが'0'の場合
    if ( decimals == 0) {
      // 一桁目のあとに.0dBを表示
      oled.print(".0dB");
    }
    // decimalsが'1'の場合
    else {
      // 一桁目のあとに.5dBを表示
      oled.print(".5dB");
    }
  }
  // データが9より大きく100より少ない場合
  else if ( (9 < level) && (level <100) ) {
    // データを100で割った余りをtensPlaceに代入
    int tensPlace = level % 100;
    // 一つのスペース
    oled.print(" ");
    // のあとに十の位を表示
    oled.print(tensPlace);
    if ( decimals == 0) {
      oled.print(".0dB");
    }
    else {
      oled.print(".5dB");
    }
  }
  // データが99より大きい場合
  else if ( level > 99 ) {
    // そのまま三桁表示
    oled.print(level);
    if ( decimals == 0) {
      oled.print(".0dB");
    }
    else {
      oled.print(".5dB");
    }
  }
}

/* サンプリング周波数の表示 */
void displayFs(uint16_t FS) {
  // 表示位置を(3.0)にする
  oled.setCursor(3, 0);
  // 再生モードがPCMの場合
  if ( DSDON == 0x00 ) {
    // サンプリング周波数が32kHzの場合
    if ( FS == 32 ) {
      oled.print(freq32);
    // サンプリング周波数が44.kHzの場合
    } else if ( FS == 44 ) {
      oled.print(freq44);
    // サンプリング周波数が48kHzの場合
    } else if ( FS == 48 ) {
      oled.print(freq48);
    // サンプリング周波数が88.2kHzの場合
    } else if ( FS == 88 ) {
      oled.print(freq88 );
    // サンプリング周波数が96kHzの場合
    } else if ( FS == 96 ) {
      oled.print(freq96);
    // サンプリング周波数が176.4kHzの場合
    } else if ( FS == 176 ) {
      oled.print(freq176);
    // サンプリング周波数が192kHzの場合
    } else if ( FS == 192 ) {
      oled.print(freq192);
    // サンプリング周波数が352.8kHzの場合
    } else if ( FS == 352 ) {
      oled.print(freq352);
    // サンプリング周波数が384kHzの場合
    } else if ( FS == 384 ) {
      oled.print(freq384);
    }
  // 再生モードがDSDで
  } else if (DSDON == 0x01 ) {
    // DSDデータストリームレートが2.8MHz(DSD64)の場合
    if ( FS == 2822 ) {
      oled.print(freqDsd64);
    // DSDデータストリームレートが5.6MHz(DSD128)の場合
    } else if ( FS == 5644 ) {
      oled.print(freqDsd128);
    // DSDデータストリームレートが11.2MHz(DSD256)の場合
    } else if ( FS == 11289 ) {
      oled.print(freqDsd256);
    // DSDデータストリームレートが22.4MHz(DSD512)の場合
    } else if ( FS == 22579 ) {
      oled.print(freqDsd512);
    }
  }
}

/* デジタルフィルタ特性の表示 */
void displayDigitalFilter(uint8_t digiFil) {
  oled.setCursor(0, 1);
  // 再生モードがDSDの場合
  if ( DSDON == 0x01 ) {
    // 表示しない
    oled.print(filterBlank);
  // 再生モードがPCMの場合で
  } else if ( DSDON == 0x00 ) {
    // digiFilが'1'の場合
    if ( digiFil == 1 ) {
      // シャープ
      oled.print(Sharp);
    // digiFilが'0'の場合     
    } else if (digiFil == 0) {
      // スロー
      oled.print(Slow);
    }
  }
}

/* 入力インターフェースの表示 */
void displayInputInterface() {
  oled.setCursor(17, 1);
  if ( (HWCNF[10] == 0x00) || (HWCNF[10] == 0x40) ){
    // 入力切替トグルスイッチのカウント値が'1'の場合
    if ( count == 1 ) {
      // USBインターフェースの選択を表示
      oled.print("[U]");
    }
    // カウント値が'2'の場合
    else if ( count == 2 ) {
      // RJ45コネクタ(LANケーブル経由のI2Sインターフェース)の選択を表示
      oled.print("[R]");
    }
    // カウント値が'3'の場合
    else if (( count == 3 ) || (count == 0)){
      // XHコネクタ(I2Sインターフェース)の選択を表示
      oled.print("[X]");
    }
    else {
      oled.print("[U]");
    }
  }
  else if ( HWCNF[10] == 0xC0 ) {
    if ( count == 1 ) {
      // USBインターフェースの選択を表示
      oled.print("[U]");
    }
    // カウント値が'2'の場合
    else if ( count == 2 ) {
      // RJ45コネクタ(LANケーブル経由のI2Sインターフェース)の選択を表示
      oled.print("[R]");
    }
    // カウント値が'3'の場合
    else if ( count == 3 ) {
      // XHコネクタ(I2Sインターフェース)の選択を表示
      oled.print("[X]");
    }
    else if ( count == 4 ) {
      // S/PDIF Opticalの選択を表示
      oled.print("[O]");
    }
    else if ( count == 5 ) {
      // S/PDIF Coaxialの選択を表示
      oled.print("[C]");
    }
    else if (( count == 6 ) || ( count == 0 )) {
      // マルチオプション基板のXHコネクタ（I2Sインターフェース）の選択を表示
      oled.print("[x]");
    }
    else
    oled.print("[U]");
  }
}
