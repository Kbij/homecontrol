#include "lib/OneWire/OneWire.h"
#include "lib/DallasTemperature/DallasTemperature.h"
#include "lib/U8glib/U8glib.h"
#include <stdlib.h>
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI 

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


int i; 

void draw(const char* temp) {
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr( 2, 18, "Temperatuur:");
  u8g.drawStr( 50, 50, temp);
  u8g.drawStr( 85,50,"\260C");
 // u8g.drawFrame(0, 0, 128, 64);
}

void setup() {
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement

  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(1);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(100,10,10);
  }
  
  pinMode(8, OUTPUT);
  i = 0;  
}

void loop() {
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  char temp[30];
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  Serial.print("Temperature for Device 1 is: ");
  Serial.println(sensors.getTempCByIndex(0)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  char tempVal[10];
  dtostrf(sensors.getTempCByIndex(0), 2, 1, tempVal);
  //floatToString(tempVal,sensors.getTempCByIndex(0),0,7,true);
  sprintf(temp, "%s", tempVal);
  
  Serial.println(temp); 
  u8g.firstPage();  
  do {
    draw(temp);
    //u8g.drawStr( 2, 32, "Temperatuur:");
    
    char buffer [33];
    itoa(i, buffer, 10);
   // u8g.setFont(u8g_font_gdr25);
  //  u8g.drawStr( 50, 50, buffer);
    //delay(500);
    
  } while( u8g.nextPage() );

  ++i;
   if (i > 32) i = 0;
  
  // rebuild the picture after some delay
  delay(2000);
}
