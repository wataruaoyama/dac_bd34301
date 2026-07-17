void initPCM9211() {
  
  /*
   * DIRリカバーシステムクロック率
   */
    i2cWrite(PCM9211_ADR, 0x30, 0x12);  // PLL SCK分周率を自動設定
    
  /*
   * XTI周波数設定
   */
   //i2cWrite(PCM9211_ADR, 0x31, 0x0A);
   
  /*
   * RXIN4ポートをOptical入力に設定
   */
  i2cWrite(PCM9211_ADR, 0x34, 0xE4);

  /*
   * 出力ポート（SCK/BCK/LRCK/DOUT）のソースの設定
   */
  i2cWrite(PCM9211_ADR, 0x6B, 0x11);  // DIRに設定
  
  /* 
   * マルチパーパス入出力ポートMPIO_A-Cの機能設定
   * MPIO_A -> Flags or GPIO 
   * MPIO_B -> Sampling Frequency Caluculated Result
   * MPIO_C -> GPIO
   */
  i2cWrite(PCM9211_ADR, 0x6F, 0xD3);

  /*
   * MPIO_A(3:0)の各ピンをGPIOに設定
   * デフォルトでは入力に設定される
   */
  i2cWrite(PCM9211_ADR, 0x70, 0x0F);

  /* 
   * MPIO_C0をGPIOに設定
   * それ以外はデフォルト（DIR Flags）
   */
  i2cWrite(PCM9211_ADR, 0x71, 0x01);

  /* 
   * マルチパーパス出力ポートの設定
   * MPO1はLVC157の有効/無効化を行う 
   * MPO0はLVDS to I2S出力かPCM9211の出力を選択する
   */
  i2cWrite(PCM9211_ADR, 0x78, 0x12);  // LVC157出力を無効化, LVDSを選択

  /* MPIO_C0 -> OUTPUT */
  i2cWrite(PCM9211_ADR, 0x7A, 0x01);  // GPIO_C0を出力ポートに設定

  /* MPIO_C0 -> HIGH */
  i2cWrite(PCM9211_ADR, 0x7C, 0x01);  // LVC541出力を無効化
}
