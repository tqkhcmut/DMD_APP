/* 
 * TimeSerial.pde
 * example code illustrating Time library set through serial port messages.
 *
 * Messages consist of the letter T followed by ten digit time (as seconds since Jan 1 1970)
 * you can send the text on the next line using Serial Monitor to set the clock to noon Jan 1 2010
   T1262347200A
 *
 * A Processing example sketch to automatically send the messages is inclided in the download
 */ 
 
#include <Time.h>
#include <TimerOne.h>
#include <SPI.h> 
#include <DMD.h>
#include "SystemFont5x7.h"
#include "Arial_black_16.h"
#include "Arial14.h"

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD DMD_Top(DISPLAYS_ACROSS, DISPLAYS_DOWN, 9);
DMD DMD_Bot(DISPLAYS_ACROSS, DISPLAYS_DOWN, 8);
byte sw = 0;

#define TIME_MSG_LEN  12   // time sync to PC is HEADER followed by unix time_t as ten ascii digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

char timeStr[] = "00-00";//:00";
#define TIME_STR_LENGTH 5
char dateStr[] = "Sat, Jan 11, 2000";
#define DATE_STR_LENGTH 17

long timerBot = 0;
long timerDMD = 0;
boolean retBot=false;

void setup()  {
  //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
  Timer1.initialize( 1600 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to DMD_Top.scanDisplayBySPI()

  //clear/init the DMD pixels held in RAM
  DMD_Top.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
  DMD_Top.selectFont(Arial_14);

  // 2
  DMD_Bot.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
  DMD_Bot.selectFont(Arial_Black_16);
  
  Serial.begin(9600);
  setSyncProvider( requestSync);  //set function to call when sync required
  Serial.println("Waiting for sync message");
  
  timerDMD=millis();
  timerBot=millis();
  retBot=false;
}

void loop(){    
  if(Serial.available() ) 
  {
    processSyncMessage();
  }
  if(timeStatus() != timeNotSet)   
  {
    if ((timerDMD+1000) < millis())
    {
      timeShow_DMD();
      timerDMD=millis();
    }
      dateShow_DMD();
  }
  else
  {
    if (retBot)
    {
      DMD_Bot.drawMarquee("Waiting for sync message",24,(32*DISPLAYS_ACROSS)-1,0);
    }
  }
  
   if ((timerBot+40) < millis()) {
     retBot=DMD_Bot.stepMarquee(-1,0);
     timerBot=millis();
   }
}

void timeShow_DMD() // look like this: "00:00"
{
  int tmp = hour();
  
  // hour
  timeStr[0] = '0' + tmp/10;
  timeStr[1] = '0' + tmp%10;
    
  // : 
  timeStr[2] = (timeStr[2] == '-') ? ' ' : '-';
  
  // minute
  tmp = minute();
  timeStr[3] = '0' + tmp/10;
  timeStr[4] = '0' + tmp%10;
  
  DMD_Top.clearScreen(true);
  DMD_Top.drawChar( 0,  3, timeStr[0], GRAPHICS_NORMAL );
  DMD_Top.drawChar( 7,  3, timeStr[1], GRAPHICS_NORMAL );
  DMD_Top.drawChar( 14, 1, timeStr[2], GRAPHICS_NORMAL );
  DMD_Top.drawChar( 19,  3, timeStr[3], GRAPHICS_NORMAL );
  DMD_Top.drawChar( 26,  3, timeStr[4], GRAPHICS_NORMAL );
}

void dateShow_DMD() // look like this: "Sat, Jan 11, 2000"
{
  int tmp = year();
  dateStr[DATE_STR_LENGTH-4] = '0' + tmp/1000;
  dateStr[DATE_STR_LENGTH-3] = '0' + (tmp/100)%10;
  dateStr[DATE_STR_LENGTH-2] = '0' + (tmp/10)%10;
  dateStr[DATE_STR_LENGTH-1] = '0' + tmp%10;
  
  tmp = day();
  dateStr[9] = '0' + tmp/10;
  dateStr[10] = '0' + tmp%10;
  
  dateStr[0] = dayShortStr(weekday())[0];
  dateStr[1] = dayShortStr(weekday())[1];
  dateStr[2] = dayShortStr(weekday())[2];
  
  dateStr[5] = monthShortStr(month())[0];
  dateStr[6] = monthShortStr(month())[1];
  dateStr[7] = monthShortStr(month())[2];
  
  if (retBot)
  {
    DMD_Bot.drawMarquee(dateStr,DATE_STR_LENGTH,(32*DISPLAYS_ACROSS)-1,0);
  }
}

void processSyncMessage() {
  // if time sync available from serial port, update time and return true
  while(Serial.available() >=  TIME_MSG_LEN ){  // time message consists of a header and ten ascii digits
    char c = Serial.read() ; 
    Serial.print(c);  
    if( c == TIME_HEADER ) {       
      time_t pctime = 0;
      for(int i=0; i < TIME_MSG_LEN -1; i++){   
        c = Serial.read();          
        if( c >= '0' && c <= '9'){   
          pctime = (10 * pctime) + (c - '0') ; // convert digits to a number    
        }
      }   
      c = Serial.read() ;   
      setTime(pctime);   // Sync Arduino clock to the time received on the serial port
    }  
  }
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);  
  return 0; // the time will be sent later in response to serial mesg
}

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
