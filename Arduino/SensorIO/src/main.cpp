#include <Arduino.h>
#include "SensorDisplay.h"
#include "FilteredTemperatureSensor.h"

SensorNs::SensorDisplay display;
SensorNs::FilteredTemperatureSensor tempSensor(1, 5);

void setup()
{
  display.init();
  display.roomName("Zolder");
  display.roomTemperature(22.3);
}

void loop()
{
  display.draw();
  delay(1000);
}
