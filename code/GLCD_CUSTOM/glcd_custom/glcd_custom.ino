/*
Pin connections:
 ------------------------------
 JHD12864E -->  ARDUINO PINS
 ------------------------------
 1     -->  GND 
 2     -->  5V 
 3     -->  
 4     -->  A3      RS
 5     -->  A2      R/W 
 6     -->  D12     E
 7     -->  D8     (DB0 of JHD)
 8     -->  D9      DB1
 9     -->  D10     DB2
 10    -->  D11     DB3
 11    -->  D4      DB4
 12    -->  D5      DB5
 13    -->  D6      DB6
 14    -->  D7      DB7
 15    -->  A0
 16    -->  A1
 17    -->  RESET
 18    -->  10k pot(other end of pot is to GND and middle terminal of pot is to pin3
 19    -->  5V through 250 ohms
 20    -->  GND
 ------------------------------
 
 digital Pin13 -> LED on GLCD module
 */
//#include "glcd_custom.h"
struct glcdBus_t{
  boolean RS;
  boolean RW;
  boolean DB0;
  boolean DB1;
  boolean DB2;
  boolean DB3;
  boolean DB4;
  boolean DB5;
  boolean DB6;
  boolean DB7;
};

const int LED_GLCD_PIN = 13;

const int GLCD_DB0_PIN = 8;
const int GLCD_DB1_PIN = 9;
const int GLCD_DB2_PIN = 10;
const int GLCD_DB3_PIN = 11;
const int GLCD_DB4_PIN = 4;
const int GLCD_DB5_PIN = 5;
const int GLCD_DB6_PIN = 6;
const int GLCD_DB7_PIN = 7;

const int GLCD_RS_PIN = 17;
const int GLCD_RW_PIN = 16;
const int GLCD_E_PIN  = 12;

glcdBus_t glcdBus;
glcdBus_t glcdReadBus;
void glcdWrite()
{
  digitalWrite(GLCD_E_PIN,  LOW);
  digitalWrite(GLCD_RW_PIN, LOW);
  digitalWrite(GLCD_RS_PIN, LOW);
  digitalWrite(GLCD_DB0_PIN, glcdBus.DB0);
  digitalWrite(GLCD_DB1_PIN, glcdBus.DB1);
  digitalWrite(GLCD_DB2_PIN, glcdBus.DB2);
  digitalWrite(GLCD_DB3_PIN, glcdBus.DB3);
  digitalWrite(GLCD_DB4_PIN, glcdBus.DB4);
  digitalWrite(GLCD_DB5_PIN, glcdBus.DB5);
  digitalWrite(GLCD_DB6_PIN, glcdBus.DB6);
  digitalWrite(GLCD_DB7_PIN, glcdBus.DB7);
  toggleGLCDEnablePin();
}

void toggleGLCDEnablePin()
{
  delayMicroseconds(5);        // pauses for 50 microseconds      
  digitalWrite(GLCD_E_PIN,  HIGH);
  delayMicroseconds(5);        // pauses for 50 microseconds      
  digitalWrite(GLCD_E_PIN,  LOW);
}

void glcdDisplayOnOff(boolean ON)
{
  glcdBus.RS = LOW;
  glcdBus.RW = LOW;
  glcdBus.DB7 = LOW;   
  glcdBus.DB6 = LOW;   
  glcdBus.DB5 = HIGH;   
  glcdBus.DB4 = HIGH;   
  glcdBus.DB3 = HIGH;   
  glcdBus.DB2 = HIGH;   
  glcdBus.DB1 = HIGH;  
  glcdBus.DB0 = ON;
  glcdWrite();
}

void glcdDataWrite(byte data)
{
  glcdBus.RS = HIGH;
  glcdBus.RW = LOW;
  glcdBus.DB7 = (0x1 & (data >> 7));   
  glcdBus.DB6 = (0x1 & (data >> 6));   
  glcdBus.DB5 = (0x1 & (data >> 5));   
  glcdBus.DB4 = (0x1 & (data >> 4));   
  glcdBus.DB3 = (0x1 & (data >> 3));   
  glcdBus.DB2 = (0x1 & (data >> 2));   
  glcdBus.DB1 = (0x1 & (data >> 1));   
  glcdBus.DB0 = (0x1 & (data >> 0));   
  glcdWrite();
}

byte glcdBusToByte(struct glcdBus_t x)
{
  byte DB;
  DB = 0;
  DB = x.DB0 ? DB | 0x01 : DB;
  DB = x.DB1 ? DB | 0x02 : DB;
  DB = x.DB2 ? DB | 0x04 : DB;
  DB = x.DB3 ? DB | 0x08 : DB;
  DB = x.DB4 ? DB | 0x10 : DB;
  DB = x.DB5 ? DB | 0x20 : DB;
  DB = x.DB6 ? DB | 0x40 : DB;
  DB = x.DB7 ? DB | 0x80 : DB;
  return DB;
}
void glcdDataRead()
{
  glcdBus.RS = HIGH;
  glcdBus.RW = HIGH;
  toggleGLCDEnablePin();
  glcdWrite();
  glcdReadBus.DB0 = digitalRead(GLCD_DB0_PIN);
  glcdReadBus.DB1 = digitalRead(GLCD_DB1_PIN);
  glcdReadBus.DB2 = digitalRead(GLCD_DB2_PIN);
  glcdReadBus.DB3 = digitalRead(GLCD_DB3_PIN);
  glcdReadBus.DB4 = digitalRead(GLCD_DB4_PIN);
  glcdReadBus.DB5 = digitalRead(GLCD_DB5_PIN);
  glcdReadBus.DB6 = digitalRead(GLCD_DB6_PIN);
  glcdReadBus.DB7 = digitalRead(GLCD_DB7_PIN);

}

void switchOFFLCD()
{
  digitalWrite(LED_GLCD_PIN, LOW); 
}

void switchONLCD()
{
  digitalWrite(LED_GLCD_PIN, HIGH); 
}

void setup() {
  pinMode(LED_GLCD_PIN, OUTPUT);
  digitalWrite(LED_GLCD_PIN, HIGH);
  Serial.begin(9600);
}

void loop()
{
  byte DB;
  if(Serial)
  {
    Serial.println("Inside loop");
  }

  switchONLCD();
  //  glcdDisplayOnOff(HIGH);
  glcdDataWrite(0x60);
  delay(1000);
  glcdDataWrite(0x70);
  delay(1000);
  glcdDataRead();
  DB = glcdBusToByte(glcdReadBus);
  if(Serial)
  {
    Serial.println(DB, HEX);
  }
  glcdDataWrite(0x65);
  delay(1000);
  glcdDataRead();
  DB = glcdBusToByte(glcdReadBus);
  if(Serial)
  {
    Serial.println(DB, HEX);
  }
}

















