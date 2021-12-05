void modeSwitch(uint16_t FS, uint8_t digiFil, uint8_t inputSource) {
  uint16_t DSD = digitalRead(DP);
//  Serial.print("prevMode = "); Serial.println(prevMode);
//  Serial.print("DSD = "); Serial.println(DSD);
  if (prevInputSource != inputSource) {
    if (DSD == 0) {
      sequenceOne();
      sequenceTwo(FS, digiFil);
      sequenceFour();
      prevMode = DSD;
    } else {
      sequenceOne();
      sequenceThree(FS);
      sequenceFive();
      prevMode = DSD;
    }
    prevInputSource = inputSource;
  }

  if ((prevMode == 0) && (DSD == 0)) {
    if ((prevPcmRate != FS) || (prevFil != digiFil)) {
      Serial.println("FS is changed!");
      sequenceOne();
      sequenceTwo(FS, digiFil);
      sequenceFour();   
    }
    prevPcmRate = FS;
    prevFil = digiFil;
  } else if ((prevMode == 1) && (DSD == 1)) {
    if (prevDsdRate != FS) {
      sequenceOne();
      sequenceThree(FS);
      sequenceFive();
    }
    prevDsdRate = FS;
  } else if ((prevMode == 0) && (DSD == 1)) {
    sequenceOne();
    sequenceThree(FS);
    sequenceFive();
    prevMode = DSD;
  } else if ((prevMode == 1) && (DSD == 0)) {
    sequenceOne();
    sequenceTwo(FS, digiFil);
    sequenceFour();
    prevMode = DSD;
  }
}

void sequenceOne() {
  uint8_t i;
  for(i=0; i<=ptrSlave; i++) {
    i2cWrite(BD34301_CHIP[i], Mute, 0x00); // ミュート　オン
    i2cWrite(BD34301_CHIP[i], DigitalPower, 0x00);  // デジタル・パワー　オフ
    i2cWrite(BD34301_CHIP[i], SoftwareReset, 0x00); // ソフト・リセット　オン
  }
}

// PCM モード時の設定
void sequenceTwo(uint16_t FS, uint8_t digiFil) {
  uint8_t i;
  for(i=0; i<=ptrSlave; i++) {
    if (FS == 32) {
      if (HWCNF[7] >= 0x08) { // Recomanded settings
        i2cWrite(BD34301_CHIP[i], Clock1, 0x03);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x01);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x02);
        if (digiFil == 1) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x80);
        else i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x83);
      } else if (HWCNF[7] == 0x04) { // Over Sampling Rate x 16
        i2cWrite(BD34301_CHIP[i], Clock1, 0x01);  // Mclk division rate is 2/3.
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x01);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x10);
        if ((digiFil == 1) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x80);
        else if ((digiFil == 1) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x00);
        else if ((digiFil == 0) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x83);
        else if ((digiFil == 0) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x03);      
      } else if (HWCNF[7] == 0x00) { // Over Sampling Rate x 32
        i2cWrite(BD34301_CHIP[i], Clock1, 0x01);  // Mclk division rate is is 2/3.
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x01);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x11);
        if ((digiFil == 1) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x80);
        else if ((digiFil == 1) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x00);
        else if ((digiFil == 0) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x83);
        else if ((digiFil == 0) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x03);        
      }
    } else if ((FS == 44) || (FS == 48)) {
      if (HWCNF[7] >= 0x08) {  // Recomanded settings
        i2cWrite(BD34301_CHIP[i], Clock1, 0x02);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x01);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x02);
        if (digiFil == 1) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x80);
        else i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x83);
      } else if (HWCNF[7] == 0x04) { // Over Sampling Rate x 16
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);  // Mclk division rate is is 1.
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x01);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x10);
        if ((digiFil == 1) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x80);
        else if ((digiFil == 1) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x00);
        else if ((digiFil == 0) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x83);
        else if ((digiFil == 0) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x03);   
      } else if (HWCNF[7] == 0x00) { // Over Sampling Rate x 32
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);  // Mclk division rate is is 1.
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x01);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x11);
        if ((digiFil == 1) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x80);
        else if ((digiFil == 1) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x00);
        else if ((digiFil == 0) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x83);
        else if ((digiFil == 0) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x03);
      }
    } else if ((FS == 88) || (FS == 96)) {
      if (HWCNF[7] >= 0x08) {  // Recomanded settings.
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x02);     
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x11);  // x 32
        if (digiFil == 1) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x01);
        else i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x04);
      } else if (HWCNF[7] == 0x04) {
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x02);     
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x10);  // x 16
        if ((digiFil == 1) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x81);
        else if ((digiFil == 1) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x01);
        else if ((digiFil == 0) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x84);
        else if ((digiFil == 0) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x04);        
      } else if (HWCNF[7] == 0x00) {
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x02);     
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x11);  // x 32        
        if ((digiFil == 1) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x81);
        else if ((digiFil == 1) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x01);
        else if ((digiFil == 0) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x84);
        else if ((digiFil == 0) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x04);
      }
    } else if ((FS == 176) || (FS == 192)) {
      if (HWCNF[7] >= 0x08) {  // Rcomanded settings
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x04);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x11);  // x 32
        if (digiFil == 1) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x02);
        else i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x05);
      } else if (HWCNF[7] == 0x04) {
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x04);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x10);  // x 16
        if ((digiFil == 1) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x82);
        else if ((digiFil == 1) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x02);
        else if ((digiFil == 0) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x85);
        else if ((digiFil == 0) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x05);     
      } else if (HWCNF[7] == 0x00) {
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x04);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x11);  // x 32        
        if ((digiFil == 1) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x82);
        else if ((digiFil == 1) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x02);
        else if ((digiFil == 0) && (HWCNF[8] == 0x04)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x05);
        else if ((digiFil == 0) && (HWCNF[8] == 0x00)) i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x85);
      }
    } else if ((FS == 352) || (FS == 384)) {
      if ((HWCNF[7] == 0x0C) || (HWCNF[7] == 0x00)) {
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x08);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x11);  // x 32
      } else if (HWCNF[7] == 0x04) {
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x08);
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x10);  // x 16
      }
      i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x80);
    } else if ((FS == 705) || (FS == 768)) {
      if (HWCNF[7] >= 0x04) {  // Recomanded settings
        i2cWrite(BD34301_CHIP[i], Clock1, 0x00);
        i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x08);    
        i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x01);  // x 16
        i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x80);
      }
    }
    if (HWCNF[9] = 0x20) i2cWrite(BD34301_CHIP[i], Clock2, 0x00);
    else i2cWrite(BD34301_CHIP[i], Clock2, 0x01);
    i2cWrite(BD34301_CHIP[i], AudioIF1, 0x0B);
    i2cWrite(BD34301_CHIP[i], Setting5, 0x16);
    i2cWrite(BD34301_CHIP[i], Setting6, 0x16);
  }
}

// DSD モード時の設定
void sequenceThree(uint16_t FS) {
  uint8_t i;
  for(i=0; i<=ptrSlave; i++) {
    i2cWrite(BD34301_CHIP[i], Clock1, 0x00);
    i2cWrite(BD34301_CHIP[i], Clock2, 0x00);
    i2cWrite(BD34301_CHIP[i], AudioIF1, 0x8B);
    if (HWCNF[4] >= 2) {
      if (FS == 2822) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x01);       // fc = 26kHz
      else if (FS == 5644) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x01);  // fc = 52kHz
      else if (FS == 11289) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x01); // fc = 104kHz
      else if (FS == 22579) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x01); // fc = 208kHz
    } else if (HWCNF[4] == 0x01) {
      if (FS == 2822) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x02);       // fc = 52kHz
      else if (FS == 5644) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x02);  // fc = 104kHz
      else if (FS == 11289) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x02); // fc = 208kHz
      else if (FS == 22579) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x02); // fc = 416kHz      
    } else if (HWCNF[4] == 0x00) {
      if (FS == 2822) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x00);       // fc = 13kHz
      else if (FS == 5644) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x00);  // fc = 26kHz
      else if (FS == 11289) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x00); // fc = 52kHz
      else if (FS == 22579) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x00); // fc = 104kHz      
    }
    i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x02);
    i2cWrite(BD34301_CHIP[i], Setting5, 0x9E);
    i2cWrite(BD34301_CHIP[i], Setting6, 0x1E);
  }
}

void sequenceFour() {
  uint8_t i;
  for(i=0; i<=ptrSlave; i++) {
    i2cWrite(BD34301_CHIP[i], SoftwareReset, 0x01);   // ソフト・リセット　オフ
    i2cWrite(BD34301_CHIP[i], DigitalPower, 0x01);    // デジタル・パワー　オン
    i2cWrite(BD34301_CHIP[i], RAMClear, 0x80);        // ラム・クリア　オン
    i2cWrite(BD34301_CHIP[i], RAMClear, 0x00);        // ラム・クリア　オフ
    i2cWrite(BD34301_CHIP[i], Mute, 0x03);            // ミュート オフ
  }
}

void sequenceFive() {
  uint8_t i;
  for(i=0; i<=ptrSlave; i++) {
    i2cWrite(BD34301_CHIP[i], SoftwareReset, 0x01);
    i2cWrite(BD34301_CHIP[i], DigitalPower, 0x01);
    i2cWrite(BD34301_CHIP[i], Mute, 0x03);
  }
}
