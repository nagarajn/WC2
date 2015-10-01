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
 DIGITAL PIN3 -> Bluetooth ON/OFF Status 
 digital Pin2 -> Interrupt from PIR sensor
 digital Pin13 -> LED on GLCD module
 
 digital Pin1 and Pin0 --> Bluetooth communication
 */
//Headers
#include "glcd.h"
#include "fonts/allFonts.h"
#include <SoftwareSerial.h>// import the serial library
#include <EEPROM.h>
#include <Wire.h>
#include "RTClib.h"
//Structures:
struct wateringTimes_t{
  uint8_t hour;
  uint8_t minute;
  //  uint8_t second;  //exculding second deliberately. does not make sense to have it so accurate.
};
struct downCounterConfig_t{
  const uint8_t MAX_COUNTER = 10;
};
//Constants:
const int  PIR_INTR_PIN         = 2; 
const int  LED_GLCD_PIN         = 13;
const int  BLUETOOTH_STATUS_PIN = 3;
const long BLUETOOTH_CONFIG_TIME_MAX = 100000UL;
const long LCD_ON_TIME = 10000UL;
const char WSET0[10] = "WSET0";
const uint8_t HOUR_DIGIT_MSB   = 7;
const uint8_t HOUR_DIGIT_LSB   = 8;
const uint8_t MINUTE_DIGIT_MSB = 9;
const uint8_t MINUTE_DIGIT_LSB = 10;
//Variables:
SoftwareSerial Bluetooth(0, 1); // RX, TX
char buffer[15];
uint8_t bufferIndex;
String BluetoothString = "";
long bluetoothConfigMillis;
char BluetoothData; // the data given from mobile
RTC_DS1307 RTC;
gText t1;
gText t2;
gText t3;
volatile long pirSensedMillis;
const int WATERING_COUNT_MAX = 4;
wateringTimes_t wateringTimes[WATERING_COUNT_MAX];
DateTime now;
downCounterConfig_t downCounterConfig;
int cmd;
////////////////////////////////////////////////
// Functions:
////////////////////////////////////////////////
void writeWateringTimesInEEPROM(uint8_t hour, uint8_t minute, uint8_t index)
{
  EEPROM.write(index*2, hour);
  EEPROM.write(index*2+1, minute);
}

//When plants are being watered:
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

//regular display. Show time and watering schedule
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
  delay(2000);
}


void loadWateringTimesFromEEPROM()
{
  for(int i=0; i<downCounterConfig.MAX_COUNTER; i++)
  {
    wateringTimes[i].hour   = EEPROM.read(i*2+0);
    wateringTimes[i].minute = EEPROM.read(i*2+1);
  }
}

void switchOFFLCD()
{
  digitalWrite(LED_GLCD_PIN, HIGH);  //TODO
}

void switchONLCD()
{
  digitalWrite(LED_GLCD_PIN, HIGH); 
}
//ISR for pir sensor
//when any activity is detected by PIR sensor, turn ON the LCD for 3min
void pirSensor()
{
  pirSensedMillis = millis();
}
int decodeCommand()
{
  if( buffer[0] == 'W' &&
    buffer[1] == 'C' &&
    buffer[2] == 'S' &&
    buffer[3] == 'E' &&
    buffer[4] == 'T')
  {
    if(buffer[5] == '0') return 0;
    if(buffer[5] == '1') return 1;
    if(buffer[5] == '2') return 2;
    if(buffer[5] == '3') return 3;
    return -1; 
  }
  else //not a command
  {
    return -1; 
  }
}

int isValidWateringTime()
{
  int tmp;
  tmp=(buffer[HOUR_DIGIT_MSB]-'0')*10+(buffer[HOUR_DIGIT_LSB]-'0');       
  if ((tmp > 23) || (tmp<0)) return 0;
  tmp=(buffer[MINUTE_DIGIT_MSB]-'0')*10+(buffer[MINUTE_DIGIT_LSB]-'0');   
  if ((tmp > 59) || (tmp<0)) return 0;
  return 1;
}
void bluetoothConfigHdrDisplay()
{
  GLCD.ClearArea();
  GLCD.CursorToXY(0,0);
  GLCD.Printf("Bluetooth Cfg Mode:\n");
  GLCD.DrawHLine(0, 8, 127);  
}
//display when connected to bluetooth device
void bluetoothConfig()
{
  GLCD.SelectFont(SystemFont5x7);
  bluetoothConfigHdrDisplay();
  while(digitalRead(BLUETOOTH_STATUS_PIN))
  {
    while(Bluetooth.available())
    {
      char character = Bluetooth.read(); // Receive a single character from the software serial port
      buffer[bufferIndex++] = character;
      if (character == '\n')
      {
        //for(int i=0; i<bufferIndex; i++)
        //  GLCD.Printf("%c", buffer[i]);
        // Add your code to parse the received line here....
        cmd = decodeCommand();
        switch(cmd)
        {
        case 0: 
        case 1:
        case 2: 
        case 3: 
          if(isValidWateringTime())
          {
            wateringTimes[cmd].hour   = (buffer[HOUR_DIGIT_MSB]  -'0')*10 + (buffer[HOUR_DIGIT_LSB]   - '0'); 
            wateringTimes[cmd].minute = (buffer[MINUTE_DIGIT_MSB]-'0')*10 + (buffer[MINUTE_DIGIT_LSB] - '0'); 
            bluetoothConfigHdrDisplay();
            GLCD.CursorToXY(0,10);
            GLCD.Printf("New Time Set:\n");
            GLCD.Printf("WCSET[%0d]: %02d:%02d\n", cmd, wateringTimes[cmd].hour, wateringTimes[cmd].minute);
            EEPROM.write((cmd*2+0), wateringTimes[cmd].hour);
            EEPROM.write((cmd*2+1), wateringTimes[cmd].minute);
          }
          else 
          {
            bluetoothConfigHdrDisplay();
            GLCD.CursorToXY(0, 10);
            GLCD.Printf("Incorrect time format!!\n");
            GLCD.Printf("Use:\n");
            GLCD.Printf("WSET<0-3> <Time>:\n");
            GLCD.Printf("Time in 24hr format:\n");
          }
          break;
        default: 
          bluetoothConfigHdrDisplay();
          GLCD.CursorToXY(0,10);
          GLCD.Printf("invld cmd"); 
        }
        // Clear receive buffer so we're ready to receive the next line
        BluetoothString = "";
        bufferIndex = 0;
      }
    }
  }
  delay(1000);
  t2.ClearArea();
  t3.ClearArea();
}

void setup() {
  //PIR Sensor stuff:
  pinMode(PIR_INTR_PIN, INPUT);
  attachInterrupt(0, pirSensor, RISING);
  pirSensedMillis = millis();
  pinMode(LED_GLCD_PIN, OUTPUT);
  digitalWrite(LED_GLCD_PIN, HIGH);
  pinMode(BLUETOOTH_STATUS_PIN, INPUT);
  GLCD.Init(NON_INVERTED);
  // put your setup code here, to run once:
  Bluetooth.begin(9600);
  Bluetooth.println("Bluetooth On please press 1 or 0 blink LED ..");
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
  if(HIGH == digitalRead(BLUETOOTH_STATUS_PIN))
  {
    bluetoothConfig();
  }
  else
  {
    normalDisplay();
    //displayDownCounter(downCounterConfig.MAX_COUNTER);
  }
}



















