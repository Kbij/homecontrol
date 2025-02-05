#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <CapacitiveSensor.h>
#include <XBee.h>
#include <U8glib.h>
#include <stdlib.h>
#include <avr/sleep.h>
#include <avr/power.h>


uint8_t channelValue[] = {0x17};
uint8_t idValue[] = { 0x12, 0x13 };
uint8_t apiModeValue[] = {0x02};

volatile int f_timer=0;
const int ONEWIRE_PIN = 2;
const int TOUCH_COMMON = 3;
const int TOUCH_UP = 4;
const int TOUCH_DOWN = 5;
const int TRANSMIT_LED = 6;
const int OLED_ENABLE = 7;
const unsigned long TEMP_INTERVAL_SECONDS = 30;
const unsigned long TEMP_MEASURE_TIME_SECONDS = 1;
const int TEMP_FILTER_LENGTH = 5; // Take the average of 3 samples;
const unsigned long MAX_TIME_AGE_MILLIS = 90000; // 90 seconds

const char MSG_SENSOR_STARTUP = '1';
const char MSG_TEMPERATURE = '2';
const char MSG_SERVER_DISCOVERY = '3';
const char MSG_SERVER_DISCOVERY_RESPONSE = '4';
const char MSG_SET_TEMPERATURE = '5';
const char MSG_SET_TEMPERATURE_UP = '6';
const char MSG_SET_TEMPERATURE_DOWN = '7';
const char MSG_SET_CALIBRATION = '8';
const char MSG_WRITE_TIME = '9';
const char MSG_HEATER_ON = 'A';
const char MSG_HEATER_OFF = 'B';

// LCD Setup
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI 
// DS18S20 Setup, use pin 2
OneWire oneWire(ONEWIRE_PIN);
DallasTemperature sensors(&oneWire);
//Software serial (debug)
SoftwareSerial debugSerial(8, 9); // RX, TX
// Touch buttons
CapacitiveSensor touchUp   = CapacitiveSensor(TOUCH_COMMON, TOUCH_UP);
CapacitiveSensor touchDown = CapacitiveSensor(TOUCH_COMMON, TOUCH_DOWN);
// create the XBee object
XBee xbee = XBee();

char ADDR_STR[17];
char SET_TEMPERATURE[10];
char ROOM_NAME[20];
char TIME[10];
float tempSet = 0;
float tempCurrent[TEMP_FILTER_LENGTH];
float dispTemp = 0;
float calibrationTemp = 0;
bool heaterOn = false;


unsigned long tempStartTime = millis();
unsigned long dispStartTime = millis();
unsigned long setSendStartTime = millis();
bool tempRequested = false;
bool bootFlag = true;
bool displaySetTemperature = false;
bool setTempReceived = true;
bool lastAcknowledged = false;
bool oledPresent = false;
unsigned long timeReceivedMillis = 0;

//At startup we are broadcasting
XBeeAddress64 sensorListener = XBeeAddress64(0x00000000, 0x0000FFFF);
ZBTxStatusResponse xbTxStatus = ZBTxStatusResponse();
ZBRxResponse xbRx = ZBRxResponse();

AtCommandRequest atRequest = AtCommandRequest((uint8_t*)"");
void blinkLed()
{
  digitalWrite(TRANSMIT_LED, LOW);
  delay(1000); 
  digitalWrite(TRANSMIT_LED, HIGH);
}

void xbeeSend(const char *format, ...)
{
  char buf[80];
  va_list ap;
  va_start(ap, format);
  vsnprintf(buf, sizeof(buf), format, ap);
  va_end(ap);

  ZBTxRequest zbString = ZBTxRequest(sensorListener, (uint8_t*)buf, strlen(buf));

  debugSerial.print("Xbee send:"); 
  debugSerial.println(buf); 
  digitalWrite(TRANSMIT_LED, LOW);
  xbee.send(zbString);
  digitalWrite(TRANSMIT_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
}

void writeMid(int vert, const char* str)
{
  int width = u8g.getStrWidth(str);  
  u8g.drawStr( (128 - width)/2, vert, str);  
}

void writeRight(int vert, const char* str)
{
  int width = u8g.getStrWidth(str);  
  u8g.drawStr( (128 - width), vert, str);  
}

void draw()
{
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);
  writeMid(10, ROOM_NAME);
  
  if (displaySetTemperature)
  {
    writeMid(30, "Set:");
    char fullTempStr[20];
    sprintf(fullTempStr, "%s \260C", SET_TEMPERATURE);
  
    writeMid(55, fullTempStr);    
  }
  else
  {
    unsigned long currentMillis = millis();

    if ((unsigned long)(currentMillis - timeReceivedMillis) < MAX_TIME_AGE_MILLIS)
    {
      writeMid(30, TIME);
    }
    else
    {
      debugSerial.print("currentMillis: ");
      debugSerial.println(currentMillis);
      writeMid(30, "      ");
    }
  
    char tempStr[4];
    dtostrf(dispTemp, 2, 1, tempStr);
    char fullTempStr[20];
    sprintf(fullTempStr, "%s \260C", tempStr);
  
    writeMid(55, fullTempStr);
	if (heaterOn)
	{
		writeRight(10, "1");
	}
	else
	{
		writeRight(10, " ");
	}
  }

  if (!lastAcknowledged)
  { 
    u8g.drawStr( 118, 10, "?");  
  }
  else if (sensorListener.getMsb() == 0 && sensorListener.getLsb() == 0x0000FFFF)
  {//We still have the broadcast address
    u8g.drawStr( 118, 10, "!");  
  } else
  {
    u8g.drawStr( 118, 10, " ");
  }

//  u8g.drawFrame(0, 0, 128, 64);
}

void sendAtCommand()
{
  // send the command
  xbee.send(atRequest);
  //Read the response, and ignore
  xbee.readPacket(1000);
}

void setXBeeName()
{
  atRequest.clearCommandValue();
  atRequest.setCommand((uint8_t*)"NI");  
  atRequest.setCommandValue((uint8_t*) ROOM_NAME);
  atRequest.setCommandValueLength(strlen(ROOM_NAME));  
  sendAtCommand();   
}

void setXBeeRetry(uint8_t retryValue)
{
  atRequest.clearCommandValue();
  atRequest.setCommand((uint8_t*)"MR");  
  atRequest.setCommandValue(&retryValue);
  atRequest.setCommandValueLength(1);  
  sendAtCommand();   
}


void setup()
{
  pinMode(TRANSMIT_LED, OUTPUT);
  pinMode(OLED_ENABLE, INPUT);
  
  blinkLed();  

  debugSerial.begin(19200);
  debugSerial.println("Sensor Init");  
  delay(2000);
  blinkLed();
  
  // start serial port
  Serial.begin(38400);
  xbee.setSerial(Serial);
  
  touchUp.set_CS_AutocaL_Millis(1000);
  touchDown.set_CS_AutocaL_Millis(1000);
  
  //Set API mode  (with escape)
  atRequest.clearCommandValue();
  atRequest.setCommand((uint8_t*)"AP");  
  atRequest.setCommandValue(apiModeValue);
  atRequest.setCommandValueLength(1);  
  sendAtCommand();
  
  atRequest.clearCommandValue();
  atRequest.setCommand((uint8_t*)"ID");  
  atRequest.setCommandValue(idValue);
  atRequest.setCommandValueLength(2);  
  sendAtCommand();

  //Setup Xbee module (DigiMesh protocol)
  atRequest.clearCommandValue();
  atRequest.setCommand((uint8_t*)"CH");  
  atRequest.setCommandValue(channelValue);
  atRequest.setCommandValueLength(1);  
  sendAtCommand();


  
  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  sensors.setResolution(12);
  
  //Get the address if the first onewire device
  uint8_t sensorAddr[8];
  sensors.getAddress(sensorAddr, 0); 
  memset(ADDR_STR, 0, sizeof(ADDR_STR));
  sprintf(ADDR_STR, "%02X%02X%02X%02X%02X%02X%02X%02X", sensorAddr[0], sensorAddr[1], sensorAddr[2], sensorAddr[3], sensorAddr[4], sensorAddr[5], sensorAddr[6], sensorAddr[7]);
  
  tempStartTime = millis();

  if (digitalRead(OLED_ENABLE) == 1)
  {
    oledPresent = true;
    debugSerial.println("Oled is present");  
  }

  if (oledPresent && (u8g.getMode() == U8G_MODE_BW ))
  {
    u8g.setColorIndex(1);
  }

  //Init temperature filter, set negative temperature
  for(int i = 0; i < TEMP_FILTER_LENGTH; ++i)
  {
    tempCurrent[i] = -1;
  }
  setXBeeRetry(3);
  /*** Configure the timer.***/
  
  /* Normal timer operation.*/
  TCCR1A = 0x00; 
  
  /* Clear the timer counter register.
   * You can pre-load this register with a value in order to 
   * reduce the timeout period, say if you wanted to wake up
   * ever 4.0 seconds exactly.
   */
  TCNT1=0x0000; 
  
  /* Configure the prescaler for 1:1024, giving us a 
   * timeout of 4.09 seconds.
   */
  TCCR1B = 0x05;
  
  /* Enable the timer overlow interrupt. */
  TIMSK1=0x01;  
}

ISR(TIMER1_OVF_vect)
{
  /* set the flag. */
   if(f_timer == 0)
   {
     f_timer = 1;
   }
}

void enterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  
  sleep_enable();


  /* Disable all of the unused peripherals. This will reduce power
   * consumption further and, more importantly, some of these
   * peripherals may generate interrupts that will wake our Arduino from
   * sleep!
   */
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_twi_disable();  

  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the timer timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}

void loop()
{
  unsigned long currentTime = millis();

  if (currentTime < tempStartTime)
  {//Time machine; overflow of long int
    tempStartTime = millis();
    currentTime = millis();
    tempRequested = false;
  }
  
  if (touchUp.capacitiveSensor(40) > 100)
  {
    if (setTempReceived || ((currentTime - setSendStartTime) > 3000)) //If response received, or more than 3 sec ago send
    {
      debugSerial.println("Up pressed");  
      xbeeSend("[6:%s]", ADDR_STR);
      setSendStartTime = millis();
      setTempReceived = false;
    }
  }
  if (touchDown.capacitiveSensor(40) > 100)
  {
    if (setTempReceived || ((currentTime - setSendStartTime) > 3000)) //If response received, or more than 3 sec ago send
    {
      debugSerial.println("Down pressed");  
      xbeeSend("[7:%s]", ADDR_STR); 
      setSendStartTime = millis();
      setTempReceived = false;
    }
  }
  
  //Request measurement of the temperature
  if (!tempRequested && (((currentTime - tempStartTime) > TEMP_INTERVAL_SECONDS * 1000ul) || bootFlag))
  {
    debugSerial.println("Request temperature");  
    tempStartTime = millis();
    sensors.requestTemperatures(); // Send the command to get temperatures
    tempRequested = true;
    bootFlag = false;
  }

  //Read the temperature
  if (tempRequested && ((currentTime - tempStartTime) > TEMP_MEASURE_TIME_SECONDS * 1000))
  {
    debugSerial.println("Read temperature");  
    tempStartTime = millis();
    tempRequested = false;

    //Move the filter values
    for(int i = (TEMP_FILTER_LENGTH - 1); i > 0  ; i--)
    {
      debugSerial.print(i);
      debugSerial.print(":");
      tempCurrent[i] = tempCurrent[i-1];
      debugSerial.println(tempCurrent[i]);
    }

    //Get the current temperature
    tempCurrent[0] = sensors.getTempCByIndex(0);
    //Use the calibration value
    tempCurrent[0] += calibrationTemp;
    
    //If we have a full filter -> calculate the average
    if (tempCurrent[TEMP_FILTER_LENGTH - 1] > 0)
    {
      debugSerial.println("Calc average");  
      float totTemp = 0;
      for(int i = 0; i < TEMP_FILTER_LENGTH; ++i)
      {
        totTemp = totTemp + tempCurrent[i];
      }
      debugSerial.print("sum:");  
      debugSerial.println(totTemp);  
      
      dispTemp = totTemp / TEMP_FILTER_LENGTH;
      debugSerial.print("disp:");  
      debugSerial.println(dispTemp);  
    }
    else
    {// Filter not full; use the current
      dispTemp = tempCurrent[0];
    }    
    
    //setXBeePowerHigh();
    
    //Send the unfiltered temperature
    char tempStr[4];
    dtostrf(tempCurrent[0], 2, 2, tempStr);
    
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
          if (xbRx.getDataLength() > 4)
          {
            if (rawData[0] == '[' && rawData[xbRx.getDataLength() - 1] == ']')
            {
              //If we received the DigiMesh address of the server
              if ((rawData[1] == MSG_SERVER_DISCOVERY_RESPONSE) && (xbRx.getDataLength() == 20))
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
              if (rawData[1] == MSG_SET_TEMPERATURE)
              {
                memset(SET_TEMPERATURE, 0, sizeof(SET_TEMPERATURE));
                memcpy(SET_TEMPERATURE, &rawData[3], xbRx.getDataLength() - 4); // len([5:])
                dispStartTime = millis();
                displaySetTemperature = true;
                setTempReceived = true;
              }

              //We received the calibration temperature & room name from the server
              if (rawData[1] == MSG_SET_CALIBRATION)
              {
                debugSerial.println("Received config");
                char configStr[30];
                memset(configStr, 0, sizeof(configStr));
                memcpy(configStr, &rawData[3], xbRx.getDataLength() - 4); // len([8:])
                String config(configStr);
                int separator = config.indexOf(':');
                if (separator > 0)
                {
                  String calStr = config.substring(0, separator);
                  calibrationTemp = calStr.toFloat();

                  String roomName = config.substring(separator + 1, config.length());
                  roomName.toCharArray(ROOM_NAME, sizeof(ROOM_NAME));

                  debugSerial.print("Room Name:");
                  debugSerial.println(ROOM_NAME);
                  
                  debugSerial.print("Calibration:");
                  debugSerial.println(calibrationTemp);
                  setXBeeName();
                }
              }

              //We received the current time from the server
              if (rawData[1] == MSG_WRITE_TIME)
              {
                memset(TIME, 0, sizeof(TIME));
                memcpy(TIME, &rawData[3], xbRx.getDataLength() - 4); // len([9:])
                timeReceivedMillis = millis();
                debugSerial.print("timeReceivedMillis: ");
                debugSerial.println(timeReceivedMillis);
              }                            
			  
              if (rawData[1] == MSG_HEATER_ON)
              {
				heaterOn = true;
                debugSerial.print("Heater on");
              } 			  
              if (rawData[1] == MSG_HEATER_OFF)
              {
				heaterOn = false;
                debugSerial.print("Heater off");
              } 			  
			  
            }
          }
        }
      }        
  }

  if (oledPresent)
  {
    u8g.firstPage();  
    do
    {
      draw();
    } while( u8g.nextPage() );
  }
  else
  {
   delay(100); 
  }
}
