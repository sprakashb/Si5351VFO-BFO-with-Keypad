char Keypad()     // returns a char from key_char table based on matrix kpd having 4k7 in rows and 1k in col with 1k going to gnd as per one wire keypad in ard cc
{
  input = analogRead(0);
  if (input < key_val[TOT_KEYS])     // no key pressed
    return ' ';
  LONG_PRESS=LOW;
  entry_time=millis();
  for (n = TOT_KEYS; n >= 0 ; n--)
  {
    if (input < key_val[n])
    {
      first_press=n;
  //    lcd.setCursor(3, 2);  // debug kpd
  //lcd.print(first_press);lcd.print(": n");lcd.print(n);

      break;
    }
  }
  delay(50);
  input = analogRead(0); // read second time to debounce
  {
    if (input < key_val[first_press])
    {
      while(analogRead(0) > key_val[TOT_KEYS])    // wait till key released
      { 
      } 
      exit_time = millis();
      if(exit_time - entry_time > 1000l ) LONG_PRESS=HIGH;
 //     lcd.setCursor(3, 3);  // debug kpd
 // lcd.print(key_char[first_press]);
 // lcd.print(input); //delay(500);
         return key_char[first_press];
    }
    else
    {
      return (' ');  // no press or error! so blank
    }
  }
}

