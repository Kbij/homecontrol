#include <Arduino.h>
#include "SensorDisplay.h"

SensorNs::SensorDisplay display;

void setup()
{
  display.init();
  display.roomName("Unknown");
  display.roomTemperature(22.3);
}

void loop()
{
  display.draw();
  delay(1000);
}
