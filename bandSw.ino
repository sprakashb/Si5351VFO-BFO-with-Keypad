void switchBands()
{
  for (int bs=0; bs<=3; bs++)
  digitalWrite(BandSwitch[bs], LOW);  // all bands off


if(bindex ==0 || bindex ==1)
digitalWrite(BandSwitch[0], HIGH); // 7-10 MHz filter activated

if(bindex ==2 || bindex ==3)
digitalWrite(BandSwitch[1], HIGH); // 14-18 MHz filter activated

if(bindex ==4 || bindex ==5 || bindex ==6)
digitalWrite(BandSwitch[2], HIGH); // 21-24-28 MHz filter activated
}
