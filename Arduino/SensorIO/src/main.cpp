#include <Arduino.h>
#include "SensorDisplay.h"
#include "FilteredTemperatureSensor.h"

const int ONEWIRE_PIN = 2;

SensorNs::SensorDisplay display;
SensorNs::FilteredTemperatureSensor<5> tempSensor(ONEWIRE_PIN);

void setup()
{
  display.init();
  display.roomName("Zolder");
  display.roomTemperature(0);
}

void loop()
{
  display.draw();
  display.roomTemperature(tempSensor.getTemperature());
  delay(1000);
}
