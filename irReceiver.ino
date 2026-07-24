/*************************************************************
  IRリモコン受信コントローラ
  ********************************
  
  Apple Reoteまたは秋月電子通商で購入可能なOptoSupplyのリモコンに対応
  
 *************************************************************/
void irReceiver() {

  checkApplePairResetPin();
  controlByIR();

}

void translateIR()
{
  uint16_t address = IrReceiver.decodedIRData.address;
  uint8_t command  = IrReceiver.decodedIRData.command;
  uint32_t rawData = IrReceiver.decodedIRData.decodedRawData;
  bool repeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;

  Serial.print("Protocol=");
  Serial.print(getProtocolString(IrReceiver.decodedIRData.protocol));

  Serial.print(" Address=0x");
  Serial.print(address, HEX);

  Serial.print(" Command=0x");
  Serial.print(command, HEX);

  Serial.print(" Raw=0x");
  Serial.print(rawData, HEX);

  if (repeat) {
    Serial.print(" REPEAT");
  }

  Serial.print(" -> ");

  if (IrReceiver.decodedIRData.protocol == APPLE) {
    switch (command) {
      case APPLE_MENU:   Serial.println("APPLE MENU"); break;
      case APPLE_PLAY:   Serial.println("APPLE PLAY/PAUSE"); break;
      case APPLE_RIGHT:  Serial.println("APPLE RIGHT"); break;
      case APPLE_LEFT:   Serial.println("APPLE LEFT"); break;
      case APPLE_UP:     Serial.println("APPLE UP"); break;
      case APPLE_DOWN:   Serial.println("APPLE DOWN"); break;
      case APPLE_CENTER: Serial.println("APPLE CENTER / OK"); break;
      default:           Serial.println("APPLE UNKNOWN"); break;
    }
  }
  else if (IrReceiver.decodedIRData.protocol == NEC &&
          IrReceiver.decodedIRData.address == OPTO_ADDR) {
    switch (IrReceiver.decodedIRData.command) {
      case OPTO_UP:       Serial.println("OptoSupply ^"); break;
      case OPTO_CENTER:   Serial.println("OptoSupply o"); break;
      case OPTO_DOWN:     Serial.println("OptoSupply v"); break;
      case OPTO_LEFT:     Serial.println("OptoSupply <-"); break;
      case OPTO_RIGHT:    Serial.println("OptoSupply ->"); break;
      case OPTO_A:        Serial.println("OptoSupply A"); break;
      case OPTO_B:        Serial.println("OptoSupply B"); break;
      case OPTO_C:        Serial.println("OptoSupply C"); break;
      case OPTO_POWER:    Serial.println("OptoSupply POWER"); break;
      default:            Serial.print("OptoSupply UNKNOWN command=0x");
                          Serial.println(IrReceiver.decodedIRData.command, HEX); break;
    }
  }
  else {
    Serial.println("UNKNOWN");
  }
}

void controlByIR()
{
  uint8_t i;
  static bool mute = false;

  if (IrReceiver.decode())
  {
    // デバッグしたい場合は有効化
    // translateIR();

    /*
      Apple Remote A1294
      Arduino-IRremoteがAPPLEプロトコルとして認識した場合
    */
    if (IrReceiver.decodedIRData.protocol == APPLE) {

      uint16_t address = IrReceiver.decodedIRData.address;
      uint8_t command = IrReceiver.decodedIRData.command;
      bool repeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;

      /*
        未ペアリング時は、最初に受信したApple Remoteを登録する
      */
      if (!applePaired && !repeat) {
        pairAppleRemote(address);
      }

      /*
        ペアリング済みで、違うAddressのApple Remoteなら無視
      */
      if (!isPairedAppleRemote(address)) {
        Serial.print("Ignored Apple Remote. Address = 0x");
        Serial.println(address, HEX);

        IrReceiver.resume();
        delay(100);
        return;
      }

      /*
        REPEAT時は直前のApple commandを使う
      */
      if (repeat) {
        command = lastAppleCommand;
      }
      else {
        lastAppleCommand = command;
      }

      /*
        CENTER/OK 長押しでApple Remoteペアリング解除
        約2秒押し続けたら解除する
      */
      if (command == APPLE_CENTER) {

        if (!appleCenterHolding) {
          appleCenterHolding = true;
          appleCenterUnpairDone = false;
          appleCenterStartTime = millis();

          Serial.println("APPLE CENTER hold start");
        }

        if (!appleCenterUnpairDone &&
            (millis() - appleCenterStartTime >= APPLE_CENTER_HOLD_TIME_MS)) {

          Serial.println("APPLE CENTER long press -> unpair");
          unpairAppleRemote();

          appleCenterUnpairDone = true;

          IrReceiver.resume();
          delay(100);
          return;
        }
      }
      else {
        appleCenterHolding = false;
        appleCenterUnpairDone = false;
      }

      /*
        音量UP/DOWN
        Apple RemoteのUP/DOWNでBD34301のATT値を増減
      */
      if ((command == APPLE_UP) || (command == APPLE_DOWN)) {

        bool doStep = false;
        uint32_t now = millis();

        if (!repeat) {
          /*
            押した瞬間。
            ここでは必ず1回だけ処理する。
          */
          doStep = true;

          irHoldCommand = command;
          irHoldStartTime = now;
          irLastStepTime = now;
        }
        else {
          /*
            REPEAT。
            押してすぐのREPEATは無視する。
            400ms以上押し続けた場合だけ、150msごとに処理する。
          */
          if ((command == irHoldCommand) &&
              (now - irHoldStartTime >= IR_HOLD_START_MS) &&
              (now - irLastStepTime >= IR_REPEAT_STEP_MS)) {

            doStep = true;
            irLastStepTime = now;
          }
        }

        if (doStep) {
          if (command == APPLE_UP) {
            if (volumeCounter > 0) {
              volumeCounter--;
            }
          }
          else {
            if (volumeCounter < 255) {
              volumeCounter++;
            }
          }
        }
      }

      /*
        入力ソースの切り替え
        Apple Remote LEFT
      */
      else if (command == APPLE_LEFT) {

        if (repeat) {
          IrReceiver.resume();
          delay(50);
          return;
        }

        irHoldCommand = 0;

        count++;

        if (HWCNF[10] == 0x00) {
          if (count == 1) {
            i2cWrite(CPLD_ADR, 0x00, 0x00);
            // digitalWrite(INSEL0, LOW);
            // digitalWrite(INSEL1, LOW);
          }
          else if (count == 2) {
            i2cWrite(CPLD_ADR, 0x00, 0x10);
            // digitalWrite(INSEL0, LOW);
            // digitalWrite(INSEL1, HIGH);
            count = 0;
          }
        }

        else if (HWCNF[10] == 0x40) {
          if (count == 1) {
            i2cWrite(CPLD_ADR, 0x00, 0x00);
            // digitalWrite(INSEL0, LOW);
            // digitalWrite(INSEL1, LOW);
          }
          else if (count == 2) {
            i2cWrite(CPLD_ADR, 0x00, 0x08);
            // digitalWrite(INSEL0, HIGH);
            // digitalWrite(INSEL1, LOW);
          }
          else if (count == 3) {
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
    }

      /*
        デジタルフィルタの切り替え
        Apple Remote RIGHT
      */
      else if (command == APPLE_RIGHT) {

        if (repeat) {
          IrReceiver.resume();
          delay(50);
          return;
        }

        irHoldCommand = 0;

        if (digiFil == 1) {
          digiFil = 0;
        }
        else {
          digiFil = 1;
        }
      }

      /*
        デジタルミュートコントロール
        Apple Remote MENU
      */
      else if (command == APPLE_MENU) {

        if (repeat) {
          IrReceiver.resume();
          delay(50);
          return;
        }

        irHoldCommand = 0;

        if (mute == true) {
          for (i = 0; i <= ptrSlave; i++) {
            i2cWrite(BD34301_CHIP[i], Mute, 0x00);  // ミュートオン
          }

          digitalWrite(pwLED, LOW);
          displayMute = true;
          mute = false;
        }
        else {
          for (i = 0; i <= ptrSlave; i++) {
            i2cWrite(BD34301_CHIP[i], Mute, 0x03);  // ミュートオフ
          }

          digitalWrite(pwLED, HIGH);
          displayMute = false;
          mute = true;
        }
      }

      /*
        PLAY/PAUSE
        現状は未使用。必要なら機能を割り当てる。
      */
      else if (command == APPLE_PLAY) {
        Serial.println("APPLE PLAY/PAUSE");
      }

      /*
        CENTER / OK
        現状は未使用。必要なら機能を割り当てる。
      */
      else if (command == APPLE_CENTER) {
        Serial.println("APPLE CENTER / OK");
        // unpairAppleRemote();
      }

      /*
        DACのATTレジスタにvolumeCounterの値を設定
      */
      for (i = 0; i <= ptrSlave; i++) {
        i2cWrite(BD34301_CHIP[i], Volume1, volumeCounter);
        i2cWrite(BD34301_CHIP[i], Volume2, volumeCounter);
      }

      IrReceiver.resume();
      delay(100);
      return;
    }

    /*
      Apple以外。
      OptoSupplyはNEC系として address / command で判定する
    */
    uint16_t address = IrReceiver.decodedIRData.address;
    uint8_t command  = IrReceiver.decodedIRData.command;
    bool repeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;

    /*
      OptoSupply以外なら無視
    */
    if (address != OPTO_ADDR) {
      Serial.print("Unknown IR: protocol=");
      Serial.print(getProtocolString(IrReceiver.decodedIRData.protocol));
      Serial.print(" address=0x");
      Serial.print(address, HEX);
      Serial.print(" command=0x");
      Serial.println(command, HEX);

      IrReceiver.resume();
      delay(100);
      return;
    }

    /*
      REPEAT時は直前のOptoSupply commandを使う
    */
    static uint8_t lastOptoCommand = 0;

    if (repeat) {
      command = lastOptoCommand;
    }
    else {
      lastOptoCommand = command;
    }

    /*  
      OptoSupply UP/DOWN
    */
    if ((command == OPTO_UP) || (command == OPTO_DOWN)) {

      bool doStep = false;
      uint32_t now = millis();

      if (!repeat) {
        doStep = true;

        irHoldCommand = command;
        irHoldStartTime = now;
        irLastStepTime = now;
      }
      else {
        if ((command == irHoldCommand) &&
            (now - irHoldStartTime >= IR_HOLD_START_MS) &&
            (now - irLastStepTime >= IR_REPEAT_STEP_MS)) {

          doStep = true;
          irLastStepTime = now;
        }
      }

      if (doStep) {
        if (command == OPTO_UP) {
          if (volumeCounter > 0) {
            volumeCounter--;
          }
        }
        else {
          if (volumeCounter < 255) {
            volumeCounter++;
          }
        }
      }
    }

    /*
      OptoSupply LEFT
      入力ソース切り替え
    */
    else if (command == OPTO_LEFT) {

      if (repeat) {
        IrReceiver.resume();
        delay(50);
        return;
      }

      irHoldCommand = 0;

      count++;

      if (HWCNF[10] == 0x00) {
        if (count == 1) {
          // digitalWrite(INSEL0, LOW);
          // digitalWrite(INSEL1, LOW);
          i2cWrite(CPLD_ADR, 0x00, 0x00); // USB
        }
        else if (count == 2) {
          // digitalWrite(INSEL0, LOW);
          // digitalWrite(INSEL1, HIGH);
          i2cWrite(CPLD_ADR, 0x00, 0x10); // XH
          count = 0;
        }
      }

      else if (HWCNF[10] == 0x40) {
        if (count == 1) {
          // digitalWrite(INSEL0, LOW);
          // digitalWrite(INSEL1, LOW);
          i2cWrite(CPLD_ADR, 0x00, 0x00); // USB
        }
        else if (count == 2) {
          // digitalWrite(INSEL0, HIGH);
          // digitalWrite(INSEL1, LOW);
          i2cWrite(CPLD_ADR, 0x00, 0x08); // RJ45
        }
        else if (count == 3) {
          // digitalWrite(INSEL0, LOW);
          // digitalWrite(INSEL1, HIGH);
          i2cWrite(CPLD_ADR, 0x00, 0x10); // XH
          count = 0;
        }
      }

      else if (HWCNF[10] == 0xC0) {
        //Serial.println("MULTI OPTION");
        // countが1の場合
        if ( count == 1 ) {
          // 入力をUSBにする
          // digitalWrite(INSEL0, LOW);
          // digitalWrite(INSEL1, LOW);
          i2cWrite(CPLD_ADR, 0x00, 0x00); // USB
        }
        else if (count == 2) {
          // 入力をRJ45コネクタ（LANケーブル経由のI2S)にする
          i2cWrite(PCM9211_ADR, 0x7C, 0x01);  // LVC541出力を無効化
          i2cWrite(PCM9211_ADR, 0x78, 0x22);  // LVC157出力を有効化、LVDSを選択
          // digitalWrite(INSEL0, HIGH);
          // digitalWrite(INSEL1, LOW);
          i2cWrite(CPLD_ADR, 0x00, 0x08); // オプションコネクタを選択
        }
        // countが3の場合
        else if (count == 3) {
          // 入力をXHコネクタ(I2S)にする
          // digitalWrite(INSEL0, LOW);
          // digitalWrite(INSEL1, HIGH);  
          i2cWrite(CPLD_ADR, 0x00, 0x10);  // XH
          // シリアルモニタに出力
          //Serial.println("XH INPUT Selected");
        }
        else if ( count == 4 ) {
          // 入力をOpticalにする
          i2cWrite(PCM9211_ADR, 0x7c, 0x01);  // LVC541出力を無効化
          i2cWrite(PCM9211_ADR, 0x34, 0xC4);  // Optical入力を選択
          i2cWrite(PCM9211_ADR, 0x78, 0x21);  // LVC157出力を有効化,PCM9211出力を選択
          // digitalWrite(INSEL0, HIGH);
          // digitalWrite(INSEL1, LOW);
          i2cWrite(CPLD_ADR, 0x00, 0x08); // オプションコネクタを選択
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
    }

    /*
      OptoSupply RIGHT
      デジタルフィルタ切り替え
    */
    else if (command == OPTO_RIGHT) {

      if (repeat) {
        IrReceiver.resume();
        delay(50);
        return;
      }

      irHoldCommand = 0;

      if (digiFil == 1) {
        digiFil = 0;
      }
      else {
        digiFil = 1;
      }
    }

    /*
      OptoSupply A
      デジタルミュート
    */
    else if (command == OPTO_A) {

      if (repeat) {
        IrReceiver.resume();
        delay(50);
        return;
      }

      irHoldCommand = 0;

      if (mute == true) {
        for (i = 0; i <= ptrSlave; i++) {
          i2cWrite(BD34301_CHIP[i], Mute, 0x00);
        }

        digitalWrite(pwLED, LOW);
        displayMute = true;
        mute = false;
      }
      else {
        for (i = 0; i <= ptrSlave; i++) {
          i2cWrite(BD34301_CHIP[i], Mute, 0x03);
        }

        digitalWrite(pwLED, HIGH);
        displayMute = false;
        mute = true;
      }
    }

    /*
      OptoSupply CENTER
      必要ならここに機能を割り当て
    */
    else if (command == OPTO_CENTER) {
      Serial.println("OptoSupply CENTER");
    }

    else {
      Serial.print("Unknown OptoSupply command: 0x");
      Serial.println(command, HEX);
    }

    /*
      DACのATTレジスタにvolumeCounterの値を設定
    */
    for (i = 0; i <= ptrSlave; i++) {
      i2cWrite(BD34301_CHIP[i], Volume1, volumeCounter);
      i2cWrite(BD34301_CHIP[i], Volume2, volumeCounter);
    }

    IrReceiver.resume();
    delay(100);
    return;
  }

  delay(50);
}

void pairAppleRemote(uint16_t address)
{
  pairedAppleAddress = address;
  applePaired = true;

  irPrefs.putBool("paired", true);
  irPrefs.putUShort("addr", pairedAppleAddress);

  Serial.print("Apple Remote paired. Address = 0x");
  Serial.println(pairedAppleAddress, HEX);
}

void unpairAppleRemote()
{
  pairedAppleAddress = 0x0000;
  applePaired = false;

  irPrefs.putBool("paired", false);
  irPrefs.putUShort("addr", 0x0000);

  Serial.println("Apple Remote unpaired.");
}

bool isPairedAppleRemote(uint16_t address)
{
  if (!applePaired) {
    return true;
  }

  return address == pairedAppleAddress;
}

// GPIO27のスイッチ押下でApple Remote ペアリングの強制解除
void checkApplePairResetPin()
{
  static bool resetDone = false;
  static uint32_t lowStartTime = 0;

  bool pinLow = (digitalRead(APPLE_PAIR_RESET_PIN) == LOW);

  if (pinLow) {
    if (lowStartTime == 0) {
      lowStartTime = millis();
    }

    if (!resetDone &&
        (millis() - lowStartTime >= PAIR_RESET_DEBOUNCE_MS)) {

      if (applePaired) {
        unpairAppleRemote();
        Serial.println("Apple Remote pairing forcibly cleared by GPIO27.");
      }
      else {
        Serial.println("Apple Remote was already unpaired.");
      }

      resetDone = true;
    }
  }
  else {
    lowStartTime = 0;
    resetDone = false;
  }
}