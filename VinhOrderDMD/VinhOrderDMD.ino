
/*--------------------------------------------------------------------------------------
  Includes
--------------------------------------------------------------------------------------*/
#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>        //
#include <TimerOne.h>   //
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD DMD_Top(DISPLAYS_ACROSS, DISPLAYS_DOWN, 9);
DMD DMD_Bot(DISPLAYS_ACROSS, DISPLAYS_DOWN, 8);
byte sw = 0;

/*--------------------------------------------------------------------------------------
  Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  called at the period set in Timer1.initialize();
--------------------------------------------------------------------------------------*/
void ScanDMD()
{ 
//  DMD_Top.scanDisplayBySPI();
  if (sw)
  {
    DMD_Top.scanDisplayBySPI();
    sw = 0;
  }
  else 
  {
    DMD_Bot.scanDisplayBySPI();
    sw = 1;
  }
}

/*--------------------------------------------------------------------------------------
  setup
  Called by the Arduino architecture before the main loop begins
--------------------------------------------------------------------------------------*/
void setup(void)
{
   //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 1600 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to DMD_Top.scanDisplayBySPI()

   //clear/init the DMD pixels held in RAM
   DMD_Top.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
   DMD_Top.selectFont(Arial_Black_16);


  // 2
  DMD_Bot.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
   DMD_Bot.selectFont(Arial_Black_16);
   
   Serial.begin(9600);
   Serial.flush();
}

/*--------------------------------------------------------------------------------------
  loop
  Arduino architecture main loop
--------------------------------------------------------------------------------------*/
void loop(void)
{
//  long start=millis();
//   long timer=start;
//   boolean ret=false;
//   
//   DMD_Top.drawMarquee("Tra Quang Kieu (^.^)",20,(32*DISPLAYS_ACROSS)-1,0);
//   while(!ret){
//     if ((timer+20) < millis()) {
//       ret=DMD_Top.stepMarquee(-1,0);
//       timer=millis();
//     }
//   }
//   
//   start=millis();
//   timer=start;
//   ret=false;
//   DMD_Bot.drawMarquee("Tra Quang Kieu (^.^)",20,(32*DISPLAYS_ACROSS)-1,0);
//   while(!ret){
//     if ((timer+20) < millis()) {
//       ret=DMD_Bot.stepMarquee(-1,0);
//       timer=millis();
//     }
//   }
   
   while (Serial.available())
   {
     char tmp = Serial.read();
     switch(tmp)
     {
       case '*':  // this is image convert program
           Serial.readBytes((char*)DMD_Top.bDMDScreenRAM, 64);
           Serial.readBytes((char*)DMD_Bot.bDMDScreenRAM, 64);
           tmp = Serial.read();
           break;
       case 'c': // this is character was sent
           DMD_Top.drawChar(  0,  3, (char)Serial.read(), GRAPHICS_NORMAL );
            break;
       case 's': // this is string marquee was sent
           {
             int length = Serial.parseInt(); // 4 byte following leter 'c' must be the length of string
           char StrBuff[length];
           Serial.readBytes(StrBuff, length);
            DMD_Top.drawMarquee(StrBuff, length, (32*DISPLAYS_ACROSS)-1,0);
             break;
           }
       default:
        break;
     }
     Serial.flush();
   }
 }

