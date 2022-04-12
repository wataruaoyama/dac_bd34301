void bootUp() {
  
  if (HWCNF[10] == 0xC0) {
    initPCM9211();
  }

  initBD34301();
  delayMicroseconds(10);
  
  uint8_t i;
  for(i=0; i<=ptrSlave; i++) {
    // ソフトリセット　オフ
    i2cWrite(BD34301_CHIP[i], SoftwareReset, 0x01);
    // デジタルパワー　オン
    i2cWrite(BD34301_CHIP[i], DigitalPower, 0x01);
    // ポップノイズ防止
    i2cWrite(BD34301_CHIP[i], Boot1, 0x6A);
    i2cWrite(BD34301_CHIP[i], Boot2, 0x10);
    i2cWrite(BD34301_CHIP[i], Boot2, 0x00);
    i2cWrite(BD34301_CHIP[i], Boot1, 0x00);  
  
    delayMicroseconds(200);
  
    // アナログパワー　オン
    i2cWrite(BD34301_CHIP[i], AnalogPower, 0x01);
    // RAMクリア　オン
    i2cWrite(BD34301_CHIP[i], RAMClear, 0x80);
    // RAMクリア　オフ
    i2cWrite(BD34301_CHIP[i], RAMClear, 0x00);
    // Mute オフ
    i2cWrite(BD34301_CHIP[i], Mute, 0x03);
  }
}
