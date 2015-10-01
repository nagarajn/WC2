/*
 */

#include "glcd.h"
#include "fonts/allFonts.h"
#include <SoftwareSerial.h>// import the serial library
SoftwareSerial Genotronex(2, 3); // RX, TX
int BluetoothData; // the data given from mobile
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;
gText t1;
gText t2;
gText t3;
int hours;
int minutes;
int seconds;
int day;
int month;
int year;
int tmpCount;
int tmpCount1;
DateTime now;

void setup() {
  GLCD.Init(NON_INVERTED);
  // put your setup code here, to run once:
  Genotronex.begin(9600);
  Genotronex.println("Bluetooth On please press 1 or 0 blink LED ..");
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    // following line sets the RTC to the date & time this sketch was compiled
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  Serial.begin(9600);
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  t1.DefineArea(textAreaTOP);
  t1.SelectFont(fixednums15x31);
  t2.DefineArea(textAreaBOTTOMLEFT);
  t2.SelectFont(SystemFont5x7);
  t2.SetFontColor(WHITE); // set font color 
  t2.ClearArea(); // Clear entire text area so you can see the full boundaries.
  t3.DefineArea(textAreaBOTTOMRIGHT);
  t3.SelectFont(SystemFont5x7);
  t3.SetFontColor(WHITE); // set font color 
  t3.ClearArea(); // Clear entire text area so you can see the full boundaries.
  tmpCount1 = 1;  
}

void displayDownCounter()
{
  GLCD.CursorToXY(0,0);
  GLCD.SelectFont(SystemFont5x7);
  GLCD.ClearArea();
  GLCD.print("Cleared");
  delay(5000);
}
void loop()
{

  now = RTC.now();
  t1.CursorToXY(0,0); // column & row is relative to text area
  hours = now.hour();
  minutes = now.minute();
  seconds = now.second();
  year = now.year();
  month = now.month();
  day = now.day();
  t3.CursorToXY(0,0); // column & row is relative to text area
  t3.Printf("%02d/%02d/%04d", day, month, year);
  t1.Printf("%02d:%02d:%02d", hours, minutes, seconds);
  Serial.print(day);
  Serial.print(":");
  Serial.print(month);
  Serial.print(":");
  Serial.println(year);
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.println(seconds);
  //t3.CursorTo(0,0); // column & row is relative to text area
  //t3.Printf("Time0: 06:00AM\n");
  //t3.Printf("Time1: 10:00AM\n");
  //    if (Genotronex.available()){
  //      BluetoothData=Genotronex.read();
  //      if(BluetoothData=='1'){   // if number 1 pressed ....
  //        t2.ClearArea();
  //        t2.Printf("ONE\n");
  //        Genotronex.println("LED  On D13 ON ! ");
  //      }
  //      else if (BluetoothData=='0'){// if number 0 pressed ....
  //        t2.ClearArea();
  //        t2.Printf("ZERO\n");
  //        Genotronex.println("LED  On D13 Off ! ");
  //      } 
  //
  //      delay(1000);
  //    }
  delay(5000);
  displayDownCounter();
}





