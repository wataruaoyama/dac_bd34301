uint8_t changeFilter() {
  bool state = digitalRead(filterSwitch);
  static bool buttonState = HIGH;
  
  if (DSDON == 0x01) oled.print(filterBlank);
  else if (digiFil == 1) {
    if ((buttonState == HIGH) && (state == LOW)) digiFil = 0;
  }
  else if (digiFil == LOW){
    if ((buttonState == HIGH) && (state == LOW)) digiFil = 1;
  }
  buttonState = state;
  return(digiFil);
}
