/*************************************************************
  ボリュームカウンタ
  ***************
  
  タイマー割り込み発生時に呼ばれる．
  ボリュームカウンタとしているが、実際にはATTカウンタである．
  volumeCounterが0でATT量は0.0dB,220で-110.0dB,255で-∞。
 *************************************************************/
void volumeControl() {
  boolean Up,Down;
  Up = digitalRead(upSwitch);
  Down = digitalRead(downSwitch);
//  volumeCounter != volumeCounter;

  // UPスイッチが押された場合
  if ( !Up ) {
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
  // Downスイッチが押された場合
  else if ( !Down ) {
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
  // DACのATTレジスタにvolumeCounterの値を設定する
  uint8_t i;
  for(i=0; i<=ptrSlave; i++){
    i2cWrite(BD34301_CHIP[i], 0x21, volumeCounter);
    i2cWrite(BD34301_CHIP[i], 0x22, volumeCounter);
  }
}
