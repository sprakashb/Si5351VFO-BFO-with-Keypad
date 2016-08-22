void doKeys()   // key decode and dispatch
{
  switch (key)
  {
    case '0':
      get_freq();
      break;
    case '1':
      sel_vfo_mem();
      break;
    case '2':
      get_mem();  // short press change mem by encoder, long - store ch (mem) data in eprom
      break;
    case '3':
      sel_band();
      break;
    case '4':
      set_sb();   // short press - select SideBand (LSB/USB), long - adj bfo freq, again long go back
      break;
    case '5':
      sel_rit();
      break;
    case '6':
      set_rit();
      break;
    case '7':
      sel_stepsize();
      break;
    case '8':
      set_if_offset();
      break;
    case '9':
      set_ssb_offset();
      break;
    case 'A':
      set_vfoA();
      break;
    case 'B':
      set_vfoB();
      break;
    case 'C':
      set_vfoC();
      break;
    case 'D':
      set_vfoD();
      break;
    case '*':
      init_memory();
      break;
    case ' ':
    default:
      return ;
  }
}
//-----------
void sel_vfo_mem()   // switch between VFO and Mem Channel mode,  '1' key
{
  VFO_mode = !VFO_mode;
  //load_freq();
}
//--------------
void get_mem()       // select memory channel  '2' key
{
  //  long kpdEntry = millis();    //time of entering this fn
  if (LONG_PRESS)  // key remains pressed > 2 sec
  {
   // lcd.setCursor(5, msgRow);
    store_freq();
    return;
  }
  // else momentary press
  if (currMode == ch_sel_mode)
  {
    //load_freq();
    currMode = displ_mode;    // if this is currently active mode make it in-active
    return;
  }
  if (!VFO_mode)
  {
    currMode = ch_sel_mode;
    modeTime = millis();
    //encoderMsg();
  }
}
//-------------
void sel_band()      // sel ham bands or continuous freq mode '3' key
{
  if (currMode == band_sel_mode) // on 2nd press the mode is deselected
  {
    currMode = displ_mode;
    return;
  }
  else
    currMode = band_sel_mode;
  modeTime = millis();
  //encoderMsg();
}
//--------------
void set_sb()   // sel sidebands lsb, usb,   '4' key selects in this sequence only
// first long press adjusts bfo freq second long press go out
{
  modeTime = millis();

  //long kpdEntry = millis();    //time of entering this fn
  if (LONG_PRESS)  // key remains pressed > 2 sec
  {
    if (currMode != bfo_adj_mode)
     {
      currMode = bfo_adj_mode;
  //    lcd.setCursor(1, msgRow);
    //  lcd.print("Adjust BFO");
     }
    else
      currMode = displ_mode;

    return;
  }
  // else if momentary press,   provision for many modes though used only 2 USB & LSB
  currSB = currSB + 1;  //currSB  0 = LSB 1 = USB  2 = CW  3 = AM  SBList[] = { "LSB", "USB", "CW", "AM"};
  if (currSB == 2)
    currSB = 0;
bfoFreq = LSBUSB[currSB];
}
//---------------------
void sel_rit()      // rit on/off  '5' key
{
  delay(200);
  ritActv = ! ritActv;
  Updated = true;
  UpdTime = millis();
}
//-------------------
void set_rit()     // rit freq entry  '6' key
{
  delay(500) ;  // repeats 6 key if not there
  currMode = rit_adj_mode;
  modeTime = millis();
  ritFreq = get_data(" Rit (* is -) ", ritFreq, 1000); // key in RIT, * key works as minus sign
}
//---------------
void sel_stepsize()      // freq change stepsize '7' key
{
  if (currMode == step_adj_mode)
  {
    currMode = displ_mode;  // deselct if already selected i.e. on second press of the key
    return;
  }
  else
    currMode = step_adj_mode;

  modeTime = millis();
  // encoderMsg();
}
//-----------------
void set_if_offset()           // set if offset for selected vfo A-D or ch 0 - 99, '8' key
{
  delay(200);
  modeTime = millis();
  currMode = if_adj_mode;
  if_offset = get_data("IF Offset", if_offset, 25000000); // max freq 15M or
}

//--------------------
void set_ssb_offset()        // set ssb offset '9' key
{
  delay(200);
  modeTime = millis();
  currMode = ssb_adj_mode;
  ssb_offset = get_data("SSB Offset", ssb_offset, 3000); // max freq 3000
}

//----------  ------------------
void get_freq()      // get freq cmd from kpd '0' key
{
  if (LONG_PRESS)  // key remains pressed > 2 sec
  {
    store_freq();
    return;
  }
  // else momentary push
  lcd.cursor();
  currMode = kpd_entry_mode;
  long num = 0;
  long mult = 10000000;
  lcd.setCursor(msgCol, msgRow);
  lcd.print("Enter freq");
  lcd.setCursor(freqCol, freqRow);
  lcd.cursor();
  delay(500); // to avoid catching 0 key again..
  key = Keypad();
  while (key != '#' && mult != 0)
  {
    switch (key)
    {
      case ' ':
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        modeTime = millis();
        num = num + (key - '0') * mult;
        lcd.print(key);
        mult = mult / 10l;
        if (mult == 100000 || mult == 100)
          lcd.print(".");
        // if(mult ==0)
        break;

      /*   case '*': // freq not -ve
        num = num * (-1);
        break; */

      case 'A':   // escape Key in case of error
        lcd.noCursor();
        cleanupmsg();
        Updated = false;
        return;
        break;
    }

    if (num > 30000000)
    {
      lcd.setCursor(msgCol, msgRow);
      lcd.print("ERROR               ");
      delay(500);
      lcd.setCursor(msgCol, msgRow);
      lcd.print("                   ");
      lcd.setCursor(msgCol, msgRow);   // blink 2 times
      lcd.print("ERROR               ");
      delay(500);
      lcd.setCursor(msgCol, msgRow);
      lcd.print("                   ");

      break;  // too big number
    }
    //  delay(200);
    key = Keypad();
  }
  currFreq = num;
  for (int i = 0; i <= 11; i++)  // chk the band and display
  {
    if (currFreq >= BandBases[i] && currFreq <= BandTops[i])
    {
      bindex = i;
      break;
    }
    Updated = true;
    UpdTime = millis();
  }
}
// - - - - -------
boolean chkPTT()
{
  boolean ptt1 = digitalRead(PTT_button);
  delay(100);  // debounce
  if ((digitalRead(PTT_button) == ptt1) && (ptt1 == LOW)) // PTT pressed then toggle PTT_out
  {
    PTT = !PTT;
    digitalWrite(PTT_out, PTT);
  }

  return PTT;
}


