/*
** Sensor Project, 2020
** Sensor
** File description:
** SensorDisplay
*/

#ifndef SENSORDISPLAY_H_
#define SENSORDISPLAY_H_
#include <Arduino.h>
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <GxEPD2_BW.h>

#pragma GCC diagnostic pop

namespace SensorNs
{

#define MAX_DISPLAY_BUFFER_SIZE 800
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

class SensorDisplay
{
public:
    SensorDisplay();
    
    void init();
    void roomName(String name);
    void roomTemperature(double temperature);
    void draw();
private:
    void writeMid(int vert, const char* str);
    void drawRoomName();
    void drawTemperature();
    GxEPD2_BW<GxEPD2_154_D67, MAX_HEIGHT(GxEPD2_154_D67)> mDisplay;
    String mRoomName;
    double mRoomTemperature;
    bool mNeedsUpdate;
}; 
}
#endif /* !SENSORDISPLAY_H_ */
