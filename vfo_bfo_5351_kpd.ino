//Arduino based dds using Si 5153 based on ArduDDS2
// nothing here is claimed my original
// Ideas of various Hams used in this set of progs,
// thanks to all those who have published their codes and various libraries for the benefit of other enthusiasts and hams.
// Copy left ... all are free to copy and use this prog for Ham radio, personal, educational or even commercial applications
// just dont forget the contribution of great people who allowed us to get benefit of their hard work.
// By : VU2SPF / SP Bhatnagar, qth at  Bhavnagar, India. PCB artwork and details also on vu2spf.blogspot.in 
// Please send error reports / suggestions/ mods to vu2spf@gmail.com
//v1.2: 24/9/2016 : minor updates in band switching
//
#include <RotaryEncoder.h>        // from  http://www.mathertel.de/Arduino/RotaryEncoderLibrary.aspx
#include <EEPROM.h>               // from arduino.cc
#include "EEPROMAnything.h"
#include <LiquidCrystal.h>        // new version from arduino.cc
#include <si5351.h>               // Etherkit - NT7S Library
#include <Wire.h>                 // for 5351
#include <SPI.h>                  // for liquid crystal lib using 595

// ------------ band inits some of these may be country / region specific, adjust accordingly
int numBands = 8; // How many bands in this program
String BandNames[] = { " 40m", " 30m", " 20m", " 17m", " 15m", " 12m", " 10m", "Cont"};  // "136k", "160m", " 80m", " 60m", may be added
int Bands[] = { 40, 30, 20, 17, 15, 12, 10, 0}; // 136, 160, 80, 60, may be added
long BandBases[] = { 7000000L, 10100000L, 14000000L, 18068000L, 21000000L, 24890000L, 28000000L, 0L}; // 135700L, 1810000L, 3500000L, 5258500L,
long BandTops[] = { 7200000L, 10150000L, 14350000L, 18168000L, 21450000L, 24990000L, 29700000L, 30000000L }; // 137800L, 2000000L, 3800000L, 5406500L,
long BandWkgFreqs[] = { 7035000L, 10106000L, 14200000L, 18086000L, 21060000L, 24906000L, 28060000L, 1000000L }; // 135700L, 1836000L, 3560000L, 5258500L,
int bindex = 0; // initial band selection index, on startup 40 m selected ,  can the last used freq  be stored on eprom and retrievd
int BandSwitch[3] = {4, 5, 6}; //digital pins which connectdriver ULN2803 to select BPF/LPF
// but total bands are 12 so we must combine a few bands together or cutdown no of bands
// eg genesis bpf and lpf are grouped as 7-10, 14-18 and 21-24-28 MHz
int prevbindex = bindex;
//----------- LCD, Encoder &  DDS defs
// LCD connections
// initialize the library with the number of the sspin (or the latch pin of the 74HC595)
LiquidCrystal lcd(10); //for 74595 based display use new lib
Si5351 si5351;  // init dds
RotaryEncoder encoder(A3, A2); // Encoder connects to A2 and A3 pins
int dir = 0; // encoder rotate dir
int pos = 0, newPos = 0;  // encoder outputs

// lcd screen layout for 20X 4 displ can have another set for 16x2
//row 0 the top line for display of VFO/mem ch band mode rit
int mcbmrRow = 0;
int memVfoCol = 0;
int chCol = 3;
int bandCol = 6;
int modeCol = 11;
int ritCol = 16;

// row 1 freq and step size
int freqRow = 1;
int freqCol = 0;
int stepCol = 16;

// messages on 3rd  row
int msgRow = 2;
int msgCol = 0;

//data entered in line 4 eg if_offset rit etc with kpd entry
int dataCol = 2;
int dataRow = 3;
int debugCol = 10; /// debug messages go here

// Keypad related
static int TOT_KEYS = 16; // for 4X4 keypad
static int key_val[17] = {1025, 590, 380, 295, 210, 175, 153, 138, 118, 105, 95, 89, 80, 72, 68, 65, 30}; // 30 for no key press, counts were read directly
static char key_char[17] = {'1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D', ' '}; // no press sends a blank
int n, input, first_press = 0;
long entry_time, exit_time;
boolean LONG_PRESS = LOW; // kepypad
char key;
int PTT_button = 2; // D2 as PTT normally high press once to activate PTT press again to deactivate
int PTT_out = 3;  // D3 as PTT control to Txcvr
boolean PTT = LOW; // PTT ON/OFF flag

// ---- freqs VFO mem ch side band variables  inits
unsigned long currFreq;
unsigned long  LSBUSB[] = {9996000UL, 9994000UL};            // BFO values tune once and change these for your rig
unsigned long deltaf[8] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000}; // freq change steps
String deltafStr[8] = { "   1", "  10", " 100", "  1k", " 10k", "100k", "  1M", " 10M"}; // to be displayed steps
int steps = 3; // initial setting 1000 Hz step change

long freq1, freq2;
long if_offset = 10000000;
int ritFreq = 600;
int currSB = 1;
String SBList[] = {  "LSB ", "USB "};
unsigned long  bfoFreq = LSBUSB[currSB];

int ssb_offset = 1500;

#define F_MIN        100000000UL               // Lower frequency limit
#define F_MAX        5000000000UL

struct all_infos {
  unsigned long currFreq_s;
  long if_offset_s;
  int ritFreq_s;
  int currSB_s;
  long bfoFreq_s;
  int ssb_offset_s;
} currInfo;

boolean VFO_mode = false;
char actvVFO = 'A'; // initial VFO selected all VFOs have initial settings of diff freqs
int currCh = 0; // working memory channel no 0 -39 (40 ch)
boolean ritActv = false;
long UpdTime; // time of last freq / ch mem/  SB / RIT updation so that last used data can be saved if not changed for 10 sec
boolean Updated = false; // flag to indicate new frq
int maxCh = 20 ; // max no of mem channels

// ===== mode of operation
enum op_modes { displ_mode, freq_adj_mode, if_adj_mode, step_adj_mode, kpd_entry_mode,  band_sel_mode, rit_adj_mode, ch_sel_mode, ssb_adj_mode, bfo_adj_mode };
int currMode = displ_mode; // initially selected mode
long modeTime = 0; // time of entering a mode for timed exit

int  VFObaseAdd = maxCh * sizeof(currInfo);  // EEPROM address where VFOs are stored

//------------- generating pulse
#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }
//--------------

void setup() {
  // currFreq = 7050000l; // test 1
  pinMode(PTT_button, INPUT);
  digitalWrite(PTT_button, HIGH);
  pinMode(PTT_out, OUTPUT); //to activate PTT in txcvr
  PTT_out = LOW;

  lcd.begin(20, 4);
  // encoder interrupts,  modify the next 2 lines if using other pins than A2 and A3
  PCICR |= (1 << PCIE1);    // This enables Pin Change Interrupt 1 that covers the Analog input pins or Port C.
  PCMSK1 |= (1 << PCINT10) | (1 << PCINT11);  // This enables the interrupt for pin 2 and 3 of Port C.

  pinMode(BandSwitch[0], OUTPUT);
  pinMode(BandSwitch[1], OUTPUT);
  pinMode(BandSwitch[2], OUTPUT);
  for (int bs = 0; bs <= 3; bs++)
    digitalWrite(BandSwitch[bs], LOW);  // all bands off


  loadLastSettings(); // initially on power up retrieve last used settings
  load_freq();
  showFreq(currFreq);
  showInfo();
  //switchBands();
  //-------------------
  Wire.begin();
  si5351.set_correction(140); //**mine. There is a calibration sketch in File/Examples/si5351Arduino-Jason
  //where you can determine the correction by using the serial monitor.

  //initialize the Si5351
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0); //If you're using a 27Mhz crystal, put in 27000000 instead of 0
  // 0 is the default crystal frequency of 25Mhz.

  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  // Set CLK0 to output the starting "vfo" frequency as set above by vfo = ?
  si5351.set_freq(currFreq, 0, SI5351_CLK0);          // SI5351_PLL_FIXED did not give any output so 0
  si5351.set_freq( bfoFreq, 0, SI5351_CLK2); // spb

  // if needed use one of these
  //si5351.drive_strength(SI5351_CLK0,SI5351_DRIVE_2MA); //you can set this to 2MA, 4MA, 6MA or 8MA
  //si5351.drive_strength(SI5351_CLK1,SI5351_DRIVE_2MA); //be careful though - measure into 50ohms
  //si5351.drive_strength(SI5351_CLK2,SI5351_DRIVE_2MA); //
} // setup
//------------------------------------------------------


void loop() {
  key = Keypad(); // look if any command key pressed
  if (key != ' ')
    doKeys();
  showFreq(currFreq);
  if (millis() - modeTime >= 5000)
    currMode = displ_mode; // to make sure that it comes out of any mode in 5 sec if not used
  showInfo();
  if ( Updated &&  (millis() - UpdTime >= 60000)) // if freq or other info changed and remains in same state for 60 sec store it on EPROM
  {
    Updated = false;
    storeLastSettings();
    store_freq();  // store the freq as per need
  }

  chkPTT();
  if (PTT)  // Tx active
    freq1 = (currFreq - if_offset + ssb_offset + ritFreq);  // rit should be added in Tx or Rx not both so detect tx/rx on pin for PTT
  else
    freq1 = (currFreq - if_offset + ssb_offset);  // RIT not added on Rx

  freq2 = abs(freq1);
  sendFrequency(freq2);
}

//----------------------

ISR(PCINT1_vect)
{
  encoder.tick();  //check encoder
  newPos = encoder.getPosition();
  if (pos == newPos)
    return;

  if (newPos > pos)
    dir = 1;
  else
    dir = -1;
  pos = newPos;

  // if interrupt generated due to encoder rotation then work according to selected current mode
  switch (currMode)
  {
    // either freq adjustment or normal display mode . change freq by step size
    case freq_adj_mode:
    case displ_mode: // if encoder rotated change freq at current step shown with ul cursor
      {
        currFreq = currFreq + (long)deltaf[steps] * dir;
        currFreq = constrain(currFreq, 100000, 30000000); // lower limit can be zero but the upper limit depends on the DDS xtal freq (1/3rd of it)
        Updated = true;
        UpdTime = millis();
        break;
      }

    // when band select key '3' is pressed
    case band_sel_mode:
      {
        bindex = bindex + dir;
        if (bindex >= numBands)  // total numBands bands defined
          bindex = 0;
        if (bindex < 0)
          bindex = numBands - 1; // index starts from 0
        currFreq = BandWkgFreqs[bindex];
        Updated = true;
        UpdTime = millis();
        break;
      }

    // Key4 long press, if encoder rotated change freq at current step shown with ul cursor
    case bfo_adj_mode:
      {
        bfoFreq = bfoFreq + (long)deltaf[steps] * dir;
        bfoFreq = constrain(bfoFreq, 50000, 25000000); // lower limit can be zero but the upper limit depends on the DDS xtal freq (1/3rd of it)
        Updated = true;
        UpdTime = millis();
        break;
      }

    // when step size key '7' pressed
    case step_adj_mode:
      {
        steps = steps + dir;
        if (steps >= 8)
          steps = 0;
        if (steps < 0)
          steps = 7;
        break;
      }

    // when mem channel select key '2' pressed ( 0 -39 = 40 ch)
    case ch_sel_mode:
      {
        //       modeTime = millis();
        currCh = currCh + dir;
        if (currCh >= maxCh)
          currCh = 0;
        if (currCh < 0)
          currCh = maxCh;
        Updated = true;
        UpdTime = millis();
        load_freq();
        break;
      }

      // showInfo(); // update the changes on display
  }
}//ISR

//----------------

void showFreq(long rx)
{ // display freq in HAM style 2 + 3 + 3 digits
  lcd.setCursor(freqCol, freqRow);
  String f( rx);  // convert number to string
  // pad if needed
  byte len = f.length();
  String padding;
  for (int i = 0; i < 7 - len; ++i)
    padding += String("0");
  f = padding + f;
  // split the frequency string
  String k, m, r;
  k = f.substring(f.length() - 3);
  m = f.substring(f.length() - 6, f.length() - 3);
  r = f.substring(0, f.length() - 6);
  if (r.length() <= 1)
    lcd.print(" ");

  // now print the frequency
  lcd.print(r);
  lcd.print('.');
  lcd.print(m);
  lcd.print('.');
  lcd.print(k);
  if (PTT == HIGH)
    lcd.print("Tx");
  else
    lcd.print("Hz");
  lcd.print(" Stp");
  lcd.print(deltafStr[steps]);

}
//--------------
void encoderMsg()   // displ indicator to rotate encoder <o>
{
  lcd.setCursor(15, msgRow);
  lcd.print("<<o>>");
}
//-------------------
long get_data(String message, long currVal, long limit)    // mesg to be displayed, current value and max data limit
{
  long num = 0;
  int   mult = 1;
  lcd.setCursor(0 , msgRow);
  lcd.print(message);
  lcd.setCursor(dataCol, dataRow);
  //lcd.print(" :");
  lcd.print(currVal);   // show current value
  lcd.cursor();
  lcd.setCursor(dataCol, dataRow);
  //delay(500);
  key = Keypad();
  while (key != '#' && num <= limit)
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
        num = num * 10 + (key - '0');
        if (num > limit)
        {
          lcd.setCursor(msgCol, msgRow);
          lcd.print("Limit Error");
          delay(1000);
          break ;
        }
        else
        {
          lcd.setCursor(dataCol, dataRow);
          lcd.print(num);
        }
        mult = mult * 10;
        break;

      case '*':
        num = num * (-1);
        lcd.setCursor(dataCol - 1, dataRow);
        lcd.print(num);
        break;

      case 'A':   // escape Key in case of error
        lcd.noCursor();
        cleanupmsg();
        Updated = false;
        return (currVal);
        break;
    }
    delay(200);
    key = Keypad();
  }
  lcd.noCursor();
  cleanupmsg();
  // to save changes
  Updated = true;
  UpdTime = millis();
  return num;
}
// -------------

void cleanupmsg()        // clean up the message and data lines
{
  // clean up lines 3 & 4
  lcd.setCursor(0 , msgRow);
  lcd.print("                    ");
  lcd.setCursor(0, dataRow);
  lcd.print("                    ");
}

//------------------
void showInfo()                // display top line infos
{
  lcd.setCursor(memVfoCol, mcbmrRow);
  if (VFO_mode)
  {
    lcd.print("VFO ");
    lcd.print(' ');
    lcd.print(actvVFO);
    lcd.print(' ');
  }
  else
  {
    lcd.print("Mem");
    if (currMode == ch_sel_mode)
      lcd.print('>');              // ready to change ch no using encoder
    else
      lcd.print(' ');

    if (currCh < 10)
      lcd.print(' ');
    lcd.print(currCh);
  }
  chkBand();  // check the correct band as per freq
  lcd.setCursor(bandCol, mcbmrRow);
  lcd.print(BandNames[bindex]);

  lcd.setCursor(modeCol, mcbmrRow);
  lcd.print(SBList[currSB]);
  lcd.print("  ");  // to fill the gap
  lcd.setCursor(ritCol, mcbmrRow);
  if (ritActv)
  {
    if (ritFreq < 0)
      lcd.print(ritFreq);
    else
    {
      if (ritFreq == 0)
        lcd.print(" 000");
      else
      {
        lcd.print(" ") ;
        lcd.print(ritFreq);
      }
    }
  }
  else
    lcd.print("     ") ;

  // 3rd and 4th row management
  switch (currMode)
  {
    case (ch_sel_mode):  // Key 2 select mem ch
      {
        lcd.setCursor(0, msgRow);
        lcd.print("Sel Mem Ch");
        encoderMsg();
        break;
      }
    case (bfo_adj_mode):
      {
        lcd.setCursor(0, msgRow);
        lcd.print("Adj BFO");
        encoderMsg();
        lcd.setCursor(dataCol, dataRow);
        lcd.print(bfoFreq);
        break;
      }
    case (band_sel_mode):
      {
        lcd.setCursor(1, msgRow);
        lcd.print("Sel Band");
        encoderMsg();
        break;
      }
    case (step_adj_mode):
      {
        lcd.setCursor(1, msgRow);
        lcd.print("Sel Step size");
        encoderMsg();
        break;
      }

    /*lcd.setCursor(debugCol, dataRow);  // debug kpd
      key = Keypad();
      lcd.print(key);*/
    /*lcd.setCursor(debugCol, dataRow); // kpd analog values
      lcd.print("     ");
      lcd.setCursor(debugCol, dataRow);
      lcd.print(analogRead(A0));*/
    // lcd.noCursor();
    case (displ_mode):
    default:
      {
        cleanupmsg();
        break;
      }

  }
}

void chkBand()
{

  for (int i = 0; i <= 11; i++)  // chk the band and display
  {
    if (currFreq >= BandBases[i] && currFreq <= BandTops[i])
    {
      bindex = i;
      if (prevbindex != bindex)
      {
        switchBands();  // ** output band selection on pins
        prevbindex = bindex;
      }
      break;
    }
  }
}
void switchBands()
{
  if (bindex == 0 || bindex == 1)
    digitalWrite(BandSwitch[0], HIGH); // 7-10 MHz filter activated
  else
    digitalWrite(BandSwitch[0], LOW); // 7-10 MHz filter off

  if (bindex == 2 || bindex == 3)
    digitalWrite(BandSwitch[1], HIGH); // 14-18 MHz filter activated
  else
    digitalWrite(BandSwitch[1], LOW); // 14-18 MHz filter off

  if (bindex == 4 || bindex == 5 || bindex == 6)
    digitalWrite(BandSwitch[2], HIGH); // 21-24-28 MHz filter activated
  else
    digitalWrite(BandSwitch[2], LOW); // 21-24-28 MHz filter off
}
//-------------
// Program DDS chip
void sendFrequency(double frequency) {
  si5351.set_freq(currFreq, 0, SI5351_CLK0);          // SI5351_PLL_FIXED did not give any output so 0
  si5351.set_freq( bfoFreq, 0, SI5351_CLK2); // spb

  // if needed use one of these
  //si5351.drive_strength(SI5351_CLK0,SI5351_DRIVE_2MA); //you can set this to 2MA, 4MA, 6MA or 8MA
  //si5351.drive_strength(SI5351_CLK1,SI5351_DRIVE_2MA); //be careful though - measure into 50ohms
  //si5351.drive_strength(SI5351_CLK2,SI5351_DRIVE_2MA); //

  //--------------

}
//----------------------









