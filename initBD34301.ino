void initBD34301() {
                                      // レジスタアドレス
//  bd34301Chip0.Clk1 = 0x02;         // 0x04
//  bd34301Chip0.Clk2 = 0x00;         // 0x06
//  bd34301Chip0.AudIF1 = 0x0B;       // 0x10
//  bd34301Chip0.AudIF2 = 0x00;       // 0x12
//  bd34301Chip0.AudIF3 = 0x00;       // 0x13
//  bd34301Chip0.AudOP = 0x01;        // 0x14
//  bd34301Chip0.DsdFil = 0x01;       // 0x16
//  bd34301Chip0.AudIP = 0x00;        // 0x17
//  bd34301Chip0.VolTrnsTime = 0x48;  // 0x20
//  bd34301Chip0.Vol1 = 0x00;         // 0x21
//  bd34301Chip0.Vol2 = 0x00;         // 0x22
//  bd34301Chip0.MutTrnsTime = 0x4B;  // 0x29
//  bd34301Chip0.FirFil1 = 0x01;      // 0x30
//  bd34301Chip0.FirFil2 = 0x80;      // 0x31
//  bd34301Chip0.Dem1 = 0x00;         // 0x33
//  bd34301Chip0.Dem2 = 0x00;         // 0x34
//  bd34301Chip0.DltSgm = 0x02;       // 0x40
//  bd34301Chip0.Set1 = 0x00;         // 0x41
//  bd34301Chip0.Set2 = 0x34;         // 0x42
//  bd34301Chip0.Set3 = 0xB8;         // 0x43
//  bd34301Chip0.Set4 = 0x0D;         // 0x48
//  bd34301Chip0.Set5 = 0x16;         // 0x60
//  bd34301Chip0.Set6 = 0x16;         // 0x61
  /**********************
   * BD34301EKV 初期設定
   **********************/
  // MCLK分週比 : 1/2
  // 出力位相調整 : なし
  // PCMモード、DSDミュート機能有効、I2S、32-bit
  // PCMステレオモード、DSDステレオモード
  // Lch、Rchスワップなし
  // Lch位相反転あり、Rch位相反転なし
  // -- DSDフィルター --
  // DSD2.8M:26kHz DSD5.6MHz:52kHz 
  // DSD11.2MHz:104kHz DSD22.4MHz:208kHz
  // 入力位相反転なし
  // ボリューム遷移時間:1024fs
  // Attenuationレベル:0dB(L,R)
  // ミュート遷移時間:1024fs
  // シャープロールオフ,高精度演算オフ
  // ディエンファシス オフ
  // オーバーサンプリングレート:8倍
  // 
  uint8_t i;
  for(i=0; i<=ptrSlave; i++) {
    /* MCLK分週比設定 */
    i2cWrite(BD34301_CHIP[i], Clock1, 0x02);
  
    /* 位相調整有無 */
    i2cWrite(BD34301_CHIP[i], Clock2, 0x00);
    
    /* オーディオデータ入力フォーマットの設定 */
    if (HWCNF[2] == 0x07) {
      i2cWrite(BD34301_CHIP[i], AudioIF1, 0x0B);  // 32-bit I2S
    } else if (HWCNF[2] == 0x06) {
      i2cWrite(BD34301_CHIP[i], AudioIF1, 0x07); // 32-bit 左詰め
    } else if (HWCNF[2] == 0x05) {
      i2cWrite(BD34301_CHIP[i], AudioIF1, 0x03); // 32-bit 右詰め
    } else if (HWCNF[2] == 0x04) {
      i2cWrite(BD34301_CHIP[i], AudioIF1, 0x02); // 24-bit 右詰め
    } else if (HWCNF[2] == 0x03) {
      i2cWrite(BD34301_CHIP[i], AudioIF1, 0x0A); // 24-bit I2S    
    } else if (HWCNF[2] == 0x02) {
      i2cWrite(BD34301_CHIP[i], AudioIF1, 0x06); // 24-bit 左詰め    
    } else if (HWCNF[2] == 0x01) {
      i2cWrite(BD34301_CHIP[i], AudioIF1, 0x08); // 16-bit I2S    
    } else if (HWCNF[2] == 0x00) {
      i2cWrite(BD34301_CHIP[i], AudioIF1, 0x00); // 16-bit 右詰め    
    }
  
    /* LRチャネルスワップ設定 */
    i2cWrite(BD34301_CHIP[i], AudioIF3, 0x00);
  
    /* DSDフィルターの選択 */
    if (HWCNF[11] == 0x01) {  // BD34301EKV
      if (HWCNF[4] >= 0x02) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x01);
      else if (HWCNF[4] == 0x00) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x00);
      else if (HWCNF[4] == 0x01) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x02);
    }
    else if (HWCNF[11] == 0x52) { // BD34352EKV
      if (HWCNF[4] >= 0x02) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x05);
      else if (HWCNF[4] == 0x00) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x04);
      else if (HWCNF[4] == 0x01) i2cWrite(BD34301_CHIP[i], DSDFilter, 0x06);
    }
  
    /* オーディオデータ入力の極性設定 */
    if (HWCNF[5] == 0x03) i2cWrite(BD34301_CHIP[i], AudioInputPolarity, 0x00);
    else if (HWCNF[5] == 0x02) i2cWrite(BD34301_CHIP[i], AudioInputPolarity, 0x01);  // Lch Inverse
    else if (HWCNF[5] == 0x01) i2cWrite(BD34301_CHIP[i], AudioInputPolarity, 0x02);  // Rch Inverse
    else if (HWCNF[5] == 0x00) i2cWrite(BD34301_CHIP[i], AudioInputPolarity, 0x03);  // L/R Inverse
    
    /* ボリューム遷移時間設定 */
    if (HWCNF[11] == 0x01)
      i2cWrite(BD34301_CHIP[i], VolumeTransitionTime, 0x48);
    else if (HWCNF[11] == 0x52)
      i2cWrite(BD34301_CHIP[i], VolumeTransitionTime, 0x78);
  
    /* アッテネーター初期値設定 */
    i2cWrite(BD34301_CHIP[i], Volume1, 0x00);
    i2cWrite(BD34301_CHIP[i], Volume2, 0x00);
  
    /* ミュート遷移時間設定 */
    i2cWrite(BD34301_CHIP[i], MuteTransitionTime, 0x08);
  
    /* FIRフィルタ設定 */
    i2cWrite(BD34301_CHIP[i], FIRFilter1, 0x01);
    i2cWrite(BD34301_CHIP[i], FIRFilter2, 0x80);

    /* ディエンファシスの設定 */
    if (HWCNF[6] == 0x00) {
      i2cWrite(BD34301_CHIP[i], DeEmphasis1, 0x02);
      i2cWrite(BD34301_CHIP[i], DeEmphasis2, 0x03); 
    }
    else {
      i2cWrite(BD34301_CHIP[i], DeEmphasis1, 0x00);
      i2cWrite(BD34301_CHIP[i], DeEmphasis2, 0x00);
    }
  
    /* ΔΣ変調の設定 */
    i2cWrite(BD34301_CHIP[i], DeltaSigma, 0x00);

    /* */
    i2cWrite(BD34301_CHIP[i], Setting1, 0x00);
    if (HWCNF[11] == 0x01) {
      i2cWrite(BD34301_CHIP[i], Setting2, 0x34);
      i2cWrite(BD34301_CHIP[i], Setting3, 0xB8);
    }
    else if (HWCNF[11] == 0x52) {
      i2cWrite(BD34301_CHIP[i], Setting2, 0x16);
      i2cWrite(BD34301_CHIP[i], Setting3, 0x2D);      
    }
    i2cWrite(BD34301_CHIP[i], Setting4, 0x0D);
    i2cWrite(BD34301_CHIP[i], Setting5, 0x16);
    i2cWrite(BD34301_CHIP[i], Setting6, 0x16);    
  }

  /* ステレオ／モノラルモード設定 */
  if (HWCNF[3] == 0xC0) { // ステレオモード時
    i2cWrite(BD34301_CHIP[0], AudioIF2, 0x00);  // Stereoモード
    /* オーディオ出力の極性設定 */
    i2cWrite(BD34301_CHIP[0], AudioOutputPolarity, 0x01); // Lch位相反転
  }
  else if (HWCNF[3] == 0x80) {  // モノモード時
    i2cWrite(BD34301_CHIP[0], AudioIF2, 0x02);  // Chip0はLch
    i2cWrite(BD34301_CHIP[1], AudioIF2, 0x03);  // Chip1はRch
    i2cWrite(BD34301_CHIP[0], AudioOutputPolarity, 0x03); // LRch位相反転
    i2cWrite(BD34301_CHIP[1], AudioOutputPolarity, 0x03); // LRch位相反転
  }

//  i2cWrite(BD34301_CHIP[i], Setting1, 0x00);
//  i2cWrite(BD34301_CHIP[i], Setting2, 0x34);
//  i2cWrite(BD34301_CHIP[i], Setting3, 0xB8);
//  i2cWrite(BD34301_CHIP[i], Setting4, 0x0D);
//  i2cWrite(BD34301_CHIP[i], Setting5, 0x16);
//  i2cWrite(BD34301_CHIP[i], Setting6, 0x16);
}
