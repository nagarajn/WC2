/*
Pin connections:
 ------------------------------
 JHD12864E -->  ARDUINO PINS
 ------------------------------
 1     -->  GND 
 2     -->  5V 
 3     -->  
 4     -->  A3
 5     -->  A2
 6     -->  D12
 7     -->  D8
 8     -->  D9
 9     -->  D10
 10    -->  D11
 11    -->  D4
 12    -->  D5
 13    -->  D6
 14    -->  D7
 15    -->  A0
 16    -->  A1
 17    -->  RESET
 18    -->  10k pot(other end of pot is to GND and middle terminal of pot is to pin3
 19    -->  5V through 250 ohms
 20    -->  GND
 ------------------------------
 
 */

#include "glcd.h"
#include "fonts/allFonts.h"
#include <SoftwareSerial.h>// import the serial library
#include <EEPROM.h>
SoftwareSerial Genotronex(2, 3); // RX, TX
int BluetoothData; // the data given from mobile
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;
gText t1;
gText t2;
gText t3;

struct wateringTimes_t{
  uint8_t hour;
  uint8_t minute;
  //  uint8_t second;  //exculding second deliberately. does not make sense to have it so accurate.
};

struct downCounterConfig_t{
  const uint8_t MAX_COUNTER = 10;
};
const int WATERING_COUNT_MAX = 4;
wateringTimes_t wateringTimes[WATERING_COUNT_MAX];
DateTime now;
downCounterConfig_t downCounterConfig;

void writeWateringTimesInEEPROM(uint8_t hour, uint8_t minute, uint8_t index)
{
  EEPROM.write(index*2, hour);
  EEPROM.write(index*2+1, minute);
}


void displayDownCounter(uint8_t maxCount)
{
  uint8_t counter;
  counter = maxCount;
  GLCD.SelectFont(fixednums15x31);
  GLCD.ClearArea();
  while(counter > 0)
  {
    if(maxCount > 99)
    {
      GLCD.CursorToXY(40,16);
      GLCD.Printf("%03d", counter);
    } 
    else
    {
      GLCD.CursorToXY(48,16);
      GLCD.Printf("%02d", counter);
    }
    counter--;
    delay(1000);
  }
  GLCD.SelectFont(Arial14);
  GLCD.ClearArea();
  GLCD.CursorToXY(12,24);
  GLCD.Printf("DONE  WATERING");
  delay(5000);
  t2.ClearArea();
  t3.ClearArea();
}


void normalDisplay()
{
  now = RTC.now();
  t1.CursorToXY(0,0); // column & row is relative to text area
  t2.CursorToXY(0,0); // column & row is relative to text area
  t3.CursorToXY(0,0); // column & row is relative to text area
  t1.Printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  t2.Printf("Schedule:\n", now.day(), now.month(), now.year());
  t2.Printf("W0: %02d:%02d\n", wateringTimes[0].hour, wateringTimes[0].minute);
  t2.Printf("W1: %02d:%02d\n", wateringTimes[1].hour, wateringTimes[1].minute);
  t2.Printf("W2: %02d:%02d\n", wateringTimes[2].hour, wateringTimes[2].minute);
  t2.Printf("W3: %02d:%02d\n", wateringTimes[3].hour, wateringTimes[3].minute);
  t3.Printf("%02d/%02d/%04d", now.day(), now.month(), now.year());
  delay(5000);  
}

void loadWateringTimesFromEEPROM()
{
  for(int i=0; i<downCounterConfig.MAX_COUNTER; i++)
  {
    wateringTimes[i].hour   = EEPROM.read(i*2+0);
    wateringTimes[i].minute = EEPROM.read(i*2+1);
  }
}

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
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
//  loadWateringTimesFromEEPROM();
//  writeWateringTimesInEEPROM(wateringTimes[3].hour, wateringTimes[3].minute, 3);

}

void loop()
{
  normalDisplay();
  displayDownCounter(downCounterConfig.MAX_COUNTER);
}







