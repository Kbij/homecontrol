
//#include <Printers.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <CapacitiveSensor.h>
#include <XBee.h>
#include <U8glib.h>
#include <stdlib.h>

// LCD Setup
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI 
const int ONEWIRE_PIN = 2;
const int TOUCH_COMMON = 3;
const int TOUCH_UP = 4;
const int TOUCH_DOWN = 5;
const int TRANSMIT_LED = 6;

// DS18S20 Setup, use pin 2
OneWire oneWire(ONEWIRE_PIN);
DallasTemperature sensors(&oneWire);

// Touch buttons
CapacitiveSensor touchUp   = CapacitiveSensor(TOUCH_COMMON, TOUCH_UP);
CapacitiveSensor touchDown = CapacitiveSensor(TOUCH_COMMON, TOUCH_DOWN);

const int TEMP_INTERVAL_SECONDS = 30;
const int TEMP_MEASURE_TIME_SECONDS = 1;

char ADDR_STR[17];
char SET_TEMPERATURE[10];
float tempSet = 0;
float tempCurrent = 0;
unsigned long tempStartTime = millis();
unsigned long dispStartTime = millis();
unsigned long setSendStartTime = millis();
bool tempRequested = false;
bool bootFlag = true;
bool displaySetTemperature = false;
bool setTempReceived = true;
bool lastAcknowledged = false;

// create the XBee object
XBee xbee = XBee();

//At startup we are broadcasting
XBeeAddress64 sensorListener = XBeeAddress64(0x00000000, 0x0000FFFF);
ZBTxStatusResponse xbTxStatus = ZBTxStatusResponse();
ZBRxResponse xbRx = ZBRxResponse();

// AT Commands
uint8_t chCmd[] = {'C','H'};
uint8_t chValue[] = {0x0B};
uint8_t idCmd[] = {'I','D'};
uint8_t idValue[] = {0x12, 0x13};

AtCommandRequest atRequest = AtCommandRequest(chCmd);

void xbeeSend(const char *format, ...)
{
  char buf[80];
  va_list ap;
  va_start(ap, format);
  vsnprintf(buf, sizeof(buf), format, ap);
  va_end(ap);

  ZBTxRequest zbString = ZBTxRequest(sensorListener, (uint8_t*)buf, strlen(buf));

  digitalWrite(TRANSMIT_LED, LOW);
  xbee.send(zbString);
  digitalWrite(TRANSMIT_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
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

  if (displaySetTemperature)
  {
    writeMid(18, "Set:");
    char fullTempStr[20];
    sprintf(fullTempStr, "%s \260C", SET_TEMPERATURE);
  
    writeMid(50, fullTempStr);    
  }
  else
  {
    writeMid(18, "Temp:");
  
    char tempStr[4];
    dtostrf(tempCurrent, 2, 1, tempStr);
    char fullTempStr[20];
    sprintf(fullTempStr, "%s \260C", tempStr);
  
    writeMid(50, fullTempStr);  
  }

  if (!lastAcknowledged)
  { 
    u8g.drawStr( 118, 15, "?");  
  }
  else if (sensorListener.getMsb() == 0 && sensorListener.getLsb() == 0x0000FFFF)
  {//We still have the broadcast address
    u8g.drawStr( 118, 15, "!");  
  } else
  {
 //   u8g.drawStr( 118, 15, " ");
  }

  u8g.drawFrame(0, 0, 128, 64);
}

void sendAtCommand()
{
  // send the command
  xbee.send(atRequest);
  //Read the response, and ignore
  xbee.readPacket(1000);
}

void setup()
{
  // start serial port
  Serial.begin(38400);
  xbee.setSerial(Serial);
  pinMode(TRANSMIT_LED, OUTPUT);
  
  touchUp.set_CS_AutocaL_Millis(1000);
  touchDown.set_CS_AutocaL_Millis(1000);

  //Setup Xbee module (DigiMesh protocol)
  atRequest.clearCommandValue();
  atRequest.setCommand(chCmd);  
  atRequest.setCommandValue(chValue);
  atRequest.setCommandValueLength(sizeof(chValue));  
  sendAtCommand();

  atRequest.clearCommandValue();
  atRequest.setCommand(idCmd);  
  atRequest.setCommandValue(idValue);
  atRequest.setCommandValueLength(sizeof(idValue));  
  sendAtCommand();
  
  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  uint8_t sensorAddr[8];
  sensors.getAddress(sensorAddr, 0);
  memset(ADDR_STR, 0, sizeof(ADDR_STR));
  sprintf(ADDR_STR, "%02X%02X%02X%02X%02X%02X%02X%02X", sensorAddr[0], sensorAddr[1], sensorAddr[2], sensorAddr[3], sensorAddr[4], sensorAddr[5], sensorAddr[6], sensorAddr[7]);
  
  tempStartTime = millis();
  /*
  if ( u8g.getMode() == U8G_MODE_BW )
  {
    u8g.setColorIndex(1);
  }
  */
}

void loop()
{
  unsigned long currentTime = millis();
  if (touchUp.capacitiveSensor(30) > 100)
  {
    if (setTempReceived || ((currentTime - setSendStartTime) > 3000)) //If response received, or more than 3 sec ago send
    {
      xbeeSend("[6:%s]", ADDR_STR);
      setSendStartTime = millis();
      setTempReceived = false;
    }
  }
  if (touchDown.capacitiveSensor(30) > 100)
  {
    if (setTempReceived || ((currentTime - setSendStartTime) > 3000)) //If response received, or more than 3 sec ago send
    {
      xbeeSend("[7:%s]", ADDR_STR); 
      setSendStartTime = millis();
      setTempReceived = false;
    }
  }
  
  if (!tempRequested && (((currentTime - tempStartTime) > TEMP_INTERVAL_SECONDS * 1000) || bootFlag))
  {
    tempStartTime = millis();
    sensors.requestTemperatures(); // Send the command to get temperatures
    tempRequested = true;
    bootFlag = false;
  }
  if (tempRequested && ((currentTime - tempStartTime) > TEMP_MEASURE_TIME_SECONDS * 1000))
  {
    tempStartTime = millis();
    tempRequested = false;
      = sensors.getTempCByIndex(0);    
    char tempStr[4];
    dtostrf(tempCurrent, 2, 2, tempStr);
    
    //If still using the broadcast address
    if (sensorListener.getMsb() == 0 && sensorListener.getLsb() == 0x0000FFFF)
    {
      xbeeSend("[3:%s]", ADDR_STR);
    }
    else
    {
      xbeeSend("[2:%s:%s]", ADDR_STR, tempStr);
    }
  }
  if (displaySetTemperature && (currentTime - dispStartTime) > 3000) //Display for 3 seconds
  {
    displaySetTemperature = false;
  }
  
  if (xbee.readPacket(50))
  {     
      if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
      {
        xbee.getResponse().getTxStatusResponse(xbTxStatus);
        lastAcknowledged = xbTxStatus.isSuccess();
      }
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
      {
        xbee.getResponse().getTxStatusResponse(xbRx);
        if (xbRx.getDataLength() > 4)
        {
          uint8_t* rawData = xbRx.getData();
          if (rawData[0] == '[' && rawData[xbRx.getDataLength() - 1] == ']')
          {
            //If we received the DigiMesh address of the server
            if ((rawData[1] == '4') && (xbRx.getDataLength() == 20))
            {
              char msbStr[10];
              char lsbStr[10];
              memset(msbStr, 0, sizeof(msbStr));
              memset(lsbStr, 0, sizeof(lsbStr));
              
              memcpy((void*)msbStr, &rawData[3], 8);
              memcpy((void*)lsbStr, &rawData[11], 8);
              uint32_t msb = strtol(msbStr, NULL, 16);
              uint32_t lsb = strtol(lsbStr, NULL, 16);
              sensorListener.setMsb(msb); 
              sensorListener.setLsb(lsb);
              xbeeSend("[1:%s]", ADDR_STR);
            }
            //We received the set temperature from the server
            if (rawData[1] == '5')
            {
              memset(SET_TEMPERATURE, 0, sizeof(SET_TEMPERATURE));
              memcpy(SET_TEMPERATURE, &rawData[3], xbRx.getDataLength() - 4); // len([5:])
              dispStartTime = millis();
              displaySetTemperature = true;
              setTempReceived = true;
            }              
          }
        }

      }        
  }
   delay(100); 
   /*
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
  */
}
