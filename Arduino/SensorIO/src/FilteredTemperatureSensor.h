/*
** Sensor Project, 2020
** Sensor
** File description:
** FilteredTemperatureSensor
*/

#ifndef FILTEREDTEMPERATURESENSOR_H_
#define FILTEREDTEMPERATURESENSOR_H_
#include <OneWire.h>
#include <DallasTemperature.h>

namespace
{
const int TEMP_MEASURE_TIME_SECONDS = 1;
}

namespace SensorNs
{

template< int FilterLength >
class FilteredTemperatureSensor
{
public:
    FilteredTemperatureSensor(int sensorPin):
        mOneWire(sensorPin),
        mSensor(&mOneWire),
        mTemperatures(),
        mCalibrationOffset(0)
    {
        for(int i = 0; i < FilterLength; ++i)
        {
            mTemperatures[i] = -1;
        }

        mSensor.begin();
        mSensor.setResolution(12);
    };

    void setCalibration(float calibration)
    {
        mCalibrationOffset = calibration;
    }

    float getTemperature()
    {
        mSensor.requestTemperatures();

        for(int i = (FilterLength - 1); i > 0  ; i--)
        {
            mTemperatures[i] = mTemperatures[i-1];
        }

        delay(TEMP_MEASURE_TIME_SECONDS * 1000);

        //Get the current temperature
        mTemperatures[0] = mSensor.getTempCByIndex(0);
        //Use the calibration value
        mTemperatures[0] += mCalibrationOffset;

        //If we have a full filter -> calculate the average
        if (mTemperatures[FilterLength - 1] > 0)
        {
            float totTemp = 0;
            for(int i = 0; i < FilterLength; ++i)
            {
                totTemp = totTemp + mTemperatures[i];
            }
        
            return totTemp / FilterLength;

        }
        else
        {// Filter not full; use the current
            return mTemperatures[0];
        }
        return 0;
    }
  
private:
    float mTemperatures[FilterLength];
    float mCalibrationOffset;
    OneWire mOneWire;//(ONEWIRE_PIN);
    DallasTemperature mSensor;//(&oneWire);
}; 
}

#endif /* !FILTEREDTEMPERATURESENSOR_H_ */
