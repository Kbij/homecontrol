/*
** Sensor Project, 2020
** Sensor
** File description:
** SensorDisplay
*/

#include "SensorDisplay.h"
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

namespace
{
uint8_t POS_ROOMNAME = 30;
uint8_t POS_TEMPERATURE = 150;
}

namespace SensorNs
{
SensorDisplay::SensorDisplay():
    mDisplay(GxEPD2_154_D67(/*CS=10*/ SS, /*DC=*/ 9, /*RST=*/ 8, /*BUSY=*/ 7)),
    mRoomName(),
    mNeedsUpdate()
{
}

void SensorDisplay::init()
{
    mDisplay.init(0);
    mDisplay.setRotation(1);
    mDisplay.setTextColor(GxEPD_BLACK);
    mDisplay.setFullWindow();
    mDisplay.firstPage();   
}

void SensorDisplay::roomName(String name)
{
    if (mRoomName != name)
    {
        mNeedsUpdate = true;
    }
    mRoomName = name;
}

void SensorDisplay::roomTemperature(float temperature)
{
    if (abs(mRoomTemperature - temperature) > 0.1)
    {
        mNeedsUpdate = true;
    }
    mRoomTemperature = temperature;
}

void SensorDisplay::writeMid(int vert, const char* str)
{
  int16_t tbx, tby; uint16_t tbw, tbh;
  mDisplay.getTextBounds(str, 0, 0, &tbx, &tby, &tbw, &tbh);
  int y = (mDisplay.width() - tbw)/2;
  mDisplay.setCursor(y, vert);
  mDisplay.print(str);
}

void SensorDisplay::drawRoomName()
{
    mDisplay.setFont(&FreeMonoBold18pt7b);
    writeMid(POS_ROOMNAME, mRoomName.c_str());
}

void SensorDisplay::drawTemperature()
{
    char tmpString[5];
    dtostrf(mRoomTemperature,4, 1, tmpString);
    mDisplay.setFont(&FreeMonoBold24pt7b);
    writeMid(POS_TEMPERATURE, tmpString);
}

void SensorDisplay::draw()
{
    if (mNeedsUpdate)
    {
        mDisplay.firstPage();  
        do
        {
            drawRoomName();
            drawTemperature();
        } while( mDisplay.nextPage());
    }

    mNeedsUpdate = false;
}
}