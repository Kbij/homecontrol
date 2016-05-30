#include "lib/OneWire/OneWire.h"
#include "lib/DallasTemperature/DallasTemperature.h"
#include "lib/U8glib/U8glib.h"
#include "lib/Touch/CapacitiveSensor.h"
#include <stdlib.h>

// LCD Setup
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI 

// DS18S20 Setup
OneWire oneWire(2);
DallasTemperature sensors(&oneWire);

// Touch buttons
CapacitiveSensor touchUp   = CapacitiveSensor(3, 4);
CapacitiveSensor touchDown = CapacitiveSensor(3, 5);

const int TEMP_INTERVAL_SECONDS = 30;
const int TEMP_MEASURE_TIME_SECONDS = 1;

char ADDR_STR[16];
float tempSet = 0;
float tempCurrent = 0;
unsigned long startTime = millis();
bool tempRequested = false;
bool bootFlag = true;

void serialPrintf(const char *format, ...)
{
  char buf[80];
  va_list ap;
  va_start(ap, format);
  vsnprintf(buf, sizeof(buf), format, ap);
  Serial.print(buf);
  va_end(ap);
}

void writeMid(int vert, const char* str)
{
  int width = u8g.getStrWidth(str);  
  u8g.drawStr( (128 - width)/2, vert, str);  
}

void draw()
{
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);

  writeMid(18, "Temp:");

  char tempStr[4];
  dtostrf(tempCurrent, 2, 1, tempStr);
  char fullTempStr[20];
  sprintf(fullTempStr, "%s \260C", tempStr);

  writeMid(50, fullTempStr);  

  u8g.drawFrame(0, 0, 128, 64);
}

void setup()
{
  // start serial port
  Serial.begin(9600);
  
  touchUp.set_CS_AutocaL_Millis(1000);
  touchDown.set_CS_AutocaL_Millis(1000);
  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  uint8_t sensorAddr[8];
  sensors.getAddress(sensorAddr, 0);
  sprintf(ADDR_STR, "%02X%02X%02X%02X%02X%02X%02X%02X", sensorAddr[0], sensorAddr[1], sensorAddr[2], sensorAddr[3], sensorAddr[4], sensorAddr[5], sensorAddr[6], sensorAddr[7]);
  serialPrintf("[%s:1]\n", ADDR_STR);
  
  startTime = millis();
  if ( u8g.getMode() == U8G_MODE_BW )
  {
    u8g.setColorIndex(1);
  }
}

void loop()
{
  unsigned long currentTime = millis();
  if (touchUp.capacitiveSensor(30) > 100)
  {
    serialPrintf("[%s:3]\n", ADDR_STR); 
  }
  if (touchDown.capacitiveSensor(30) > 100)
  {
    serialPrintf("[%s:4]\n", ADDR_STR); 
  }
  
  if (!tempRequested && (((currentTime - startTime) > TEMP_INTERVAL_SECONDS * 1000) || bootFlag))
  {
    startTime = millis();
    sensors.requestTemperatures(); // Send the command to get temperatures
    tempRequested = true;
    bootFlag = false;
  }
  if (tempRequested && ((currentTime - startTime) > TEMP_MEASURE_TIME_SECONDS * 1000))
  {
    startTime = millis();
    tempRequested = false;
    tempCurrent = sensors.getTempCByIndex(0);    
    char tempStr[4];
    dtostrf(tempCurrent, 2, 1, tempStr);
    serialPrintf("[%s:2:%s]\n", ADDR_STR, tempStr);
  }
 
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );

  // rebuild the picture after some delay
  delay(50);
}
