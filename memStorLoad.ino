
void store_freq()
{
  int epromAddress = 0; //The base address
  currInfo.currFreq_s = currFreq;
  currInfo.if_offset_s = if_offset;
  currInfo.ritFreq_s = ritFreq;
  currInfo.currSB_s = currSB;
  currInfo.bfoFreq_s = bfoFreq ;
  currInfo.ssb_offset_s = ssb_offset;

  if (!VFO_mode)
    epromAddress = epromAddress + currCh * sizeof(currInfo); // in mem mode ch no x  n bytes info for each ch
  else
  {
    switch (actvVFO)
    {
      case 'A':
        epromAddress = VFObaseAdd; // base address for A VFO
        break;

      case 'B':
        epromAddress = VFObaseAdd + sizeof(currInfo);
        break;

      case 'C':
        epromAddress = VFObaseAdd + 2 * sizeof(currInfo);
        break;

      case 'D':
        epromAddress = VFObaseAdd + 3 * sizeof(currInfo);
        break;
    }
  }

  EEPROM_writeAnything(epromAddress, currInfo);
  lcd.setCursor(msgCol, msgRow);
  lcd.print("Stored");
  delay(500);
}

//----------------
void load_freq()
{
  int epromAddress = 0; //The base address
  if (!VFO_mode)
    epromAddress = epromAddress + currCh * sizeof(currInfo); // ch no x n bytes info for each ch
  else
  {
    switch (actvVFO)
    {
      case 'A':
        epromAddress = VFObaseAdd; // base address for A VFO
        break;

      case 'B':
        epromAddress = VFObaseAdd + sizeof(currInfo);
        break;

      case 'C':
        epromAddress = VFObaseAdd + 2 * sizeof(currInfo);
        break;

      case 'D':
        epromAddress = VFObaseAdd + 3 * sizeof(currInfo);
        break;
    }
  }
  EEPROM_readAnything( epromAddress, currInfo);
    if (currInfo.currFreq_s <1000000ul || currInfo.currFreq_s > 30000000ul)
    currFreq = 7051000ul;
    else
    currFreq = currInfo.currFreq_s;
    
  if_offset = currInfo.if_offset_s;
  ritFreq = currInfo.ritFreq_s;
  currSB = currInfo.currSB_s;
  bfoFreq = currInfo.bfoFreq_s;
  ssb_offset = currInfo.ssb_offset_s;
}

void storeLastSettings()
{ //chk for a differnt setting before storing
  int epromAddress = VFObaseAdd + 4 * sizeof(currInfo);
  if (VFO_mode == EEPROM.read(epromAddress))
  {
    epromAddress++;
    if ( actvVFO == EEPROM.read(epromAddress))
    {
      epromAddress++;
      if ( currCh == EEPROM.read(epromAddress))
        return;  // if all three are same otherwise store
    }
  }

epromAddress = VFObaseAdd + 4 * sizeof(currInfo);
EEPROM.write(epromAddress, VFO_mode);
epromAddress++;
EEPROM.write(epromAddress, actvVFO);
epromAddress++;
EEPROM.write(epromAddress, currCh);
lcd.setCursor(msgCol, msgRow);
lcd.print("Stored");
delay(500);
}

void loadLastSettings()
{
  int epromAddress = VFObaseAdd + 4 * sizeof(currInfo);
  VFO_mode = EEPROM.read(epromAddress);
  epromAddress++;
  actvVFO = EEPROM.read(epromAddress);
  epromAddress++;
  currCh = EEPROM.read(epromAddress);
}

void init_memory()   // to be done once at startup to store initial settings else it will overwrite saved info 
{
  lcd.setCursor(msgCol, msgRow);
  lcd.print("Press # to init mem");
  char c = Keypad();
  while (c == ' ')
    c = Keypad(); // wait for a charcter
  if (c != '#')
    return;
  lcd.setCursor(msgCol, msgRow);
  lcd.print("                    ");

  for (int chno = 0 ; chno <= maxCh+4; chno++)
  {
    //  lcd.setCursor(msgCol, msgRow);
    //  lcd.print("init memory ");
    lcd.print(chno);
    currCh = chno;
    currFreq = 7050000ul;
    if_offset = 10000000l;
    ritFreq = 0;
    currSB = 0;
    bfoFreq = 9996000UL;
    ssb_offset = 1500;
    store_freq();
    int currChOld = currCh;
    unsigned long currFreqOld = currFreq;
    long if_offsetOld = if_offset;
    int ritFreqOld = ritFreq;
    int currSBOld = currSB;

    load_freq();
    if (currChOld != currCh)
    {
      lcd.setCursor(msgCol, msgRow);
      lcd.print("Error currCh ");
    }
    if (currFreqOld != currFreq)
    {
      lcd.setCursor(msgCol, msgRow);
      lcd.print("Error currFreq ");
    }
    if (if_offsetOld != if_offset)
    {
      lcd.setCursor(msgCol, msgRow);
      lcd.print("Error if_offset ");
    }


  }
  cleanupmsg();
  currCh = 0;
}
