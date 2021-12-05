/*************************************************************
  システム立ち上がり時のディスプレイ表示
  ********************************
  
  システムの電源オン時にシステム構成、設定内容の表示を行う．
  表示内容は
  1.使用DACと構成（ステレオ／デュアルモノ）
  2.Audio InterfaceフォーマットとDSD playback path
  3.ゲイン設定とディエンファシスおよびDSDフィルタのカットオフ設定
  で、各3秒間表示する．
  
 *************************************************************/
void initSO2002A() {

//  oled.begin(20, 2);
//  oled.clear();
  oled.setCursor(0,0);
  
  /* 最初の3秒間の表示 */
  
  if ( HWCNF[0] == 0x07 ) oled.print(ak4499);
  else if ( HWCNF[0] == 0x06 ) oled.print(ak4493);
  else if ( HWCNF[0] == 0x05 ) oled.print(ak4495);
  else if ( HWCNF[0] == 0x04 ) oled.print(ak4490);
  else if ( HWCNF[0] == 0x03 ) oled.print(bd34301);
  else if ( HWCNF[0] == 0x01 ) oled.print(es9038q);
  else oled.print(others);
  
  oled.setCursor(7,0);
  // ステレオモードの設定
  if ( HWCNF[3] == 0xC0 ) oled.print(" Stereo");
  // デュアル・モノモード設定の場合
  else if ( HWCNF[3] == 0x80 ) oled.print(" Dual Mono");
  
  oled.setCursor(3,1);
  oled.print("by LINUXCOM");
  delay(3000);  
  oled.clear();

  /* 2回目の3秒間の表示 */
  // 設定されたオーディオインターフェースフォーマットの表示
  oled.setCursor(0,0);
  oled.print("DIF:");
  oled.setCursor(4,0);
  if ( HWCNF[2] == 0x38 ) oled.print(audioIF7); // 32bit I2Sを表示
  else if ( HWCNF[2] == 0x30 ) oled.print(audioIF6);  // 32bit MSBを表示
  else if ( HWCNF[2] == 0x28 ) oled.print(audioIF5);  // 32bit LSBを表示
  else if ( HWCNF[2] == 0x20 ) oled.print(audioIF4);  // 24bit LSBを表示
  else if ( HWCNF[2] == 0x18 ) oled.print(audioIF3);  // 24bit I2Sを表示
  else if ( HWCNF[2] == 0x10 ) oled.print(audioIF2);  // 24bit MSBを表示
  else if ( HWCNF[2] == 0x08 ) oled.print(audioIF8);  // 16bit I2Sを表示
  else if ( HWCNF[2] == 0x00 ) oled.print(audioIF0);  // 16bit LSBを表示

  /* */
//  oled.setCursor(0,1);
//  oled.print("INPOL:");
//  oled.setCursor(6,1);
//  if ( HWCNF[5] == 0x03 ) oled.print("LP RP -- --");
//  if ( HWCNF[5] == 0x02 ) oled.print("LN RP -- --");
//  if ( HWCNF[5] == 0x01 ) oled.print("LP RN -- --");
//  if ( HWCNF[5] == 0x00 ) oled.print("LN RN -- --");

  /* FIR高精度演算の設定表示 */
  oled.setCursor(0,1);
  oled.print("HPC:");
  oled.setCursor(4,1);
  if (HWCNF[7] >= 0x08) oled.print(demOff);
  else if (HWCNF[8] == 0x04) oled.print(demOff);
  else if (HWCNF[8] == 0x00) oled.print(demOn);

  /* 内部クロック位相調整の設定表示 */
  oled.setCursor(8,1);
  oled.print("PAC:");
  if (HWCNF[7] >= 0x08) oled.print(demOff);
  else if (HWCNF[9] == 0x20) oled.print(demOff);
  else if (HWCNF[9] == 0x00) oled.print(demOn);
  delay(3000);
  oled.clear();
  
  /* 3回目の3秒間の表示 */
  // DSDフィルタのカットオフ周波数設定表示
  oled.setCursor(0,0);
  oled.print("DSDF:");
  if ( HWCNF[4] >= 0x02 ) oled.print("26/52/104/208");
  else if ( HWCNF[4] == 0x00) oled.print("13/26/52/104");
  else if ( HWCNF[4] == 0x01 ) oled.print("52/104/208/416");

  // ディエンファシスの設定内容表示
  oled.setCursor(0,1);
  oled.print("DEM:");
  oled.setCursor(4,1);
  if ( HWCNF[6] == 0x10 ) oled.print(demOff); // ディエンファシスがオフ
  else oled.print(demOn); // ディエンファシスがオン

  /* オーバーサンプリングレートの設定表示 */
  oled.setCursor(8,1);
  oled.print("OSR:");
  if (HWCNF[7] >= 0x08) oled.print("REC.");
  else if (HWCNF[7] == 0x04) oled.print("x16");
  else if (HWCNF[7] == 0x00) oled.print("x32");
  delay(3000);
  oled.clear();
}
