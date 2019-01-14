/*
 * TemperatureFilter.cpp
 *
 *  Created on: Jun 1, 2016
 *      Author: koen
 */

#include <Logic/TemperatureFilter.h>
#include <numeric>
#include <glog/logging.h>
namespace
{
const int SAMPLE_COUNT = 30;
}

namespace LogicNs {

TemperatureFilter::TemperatureFilter(CommNs::TemperatureSourceIf* source, double alpha):
	mSource(source),
	mDataMutex(),
	mListeners(),
	mAlpha(alpha),
	mFilterValues()
{
	if (mSource)
	{
		mSource->registerTemperatureListener(this);
	}
}

TemperatureFilter::~TemperatureFilter()
{
	if (mSource)
	{
		mSource->unRegisterTemperatureListener(this);
	}
}

void TemperatureFilter::sensorStarted(const std::string& sensorId)
{
	std::lock_guard<std::mutex> lock(mDataMutex);

	for(auto listener: mListeners)
	{
		listener->sensorStarted(sensorId);
	}
}

void TemperatureFilter::sensorTemperature(const std::string& sensorId, double temperature)
{
	std::lock_guard<std::mutex> lock(mDataMutex);
	double sendValue = temperature;
	mFilterValues[sensorId].mValue = (mAlpha * temperature) + (1.0 - mAlpha) * mFilterValues[sensorId].mValue;

	if (mFilterValues[sensorId].mStarted)
    {
    	sendValue =  mFilterValues[sensorId].mValue;
    }
	else
	{
    	LOG(WARNING) << "Filter not stable: " << sensorId << ", diff: " << fabs(temperature - mFilterValues[sensorId].mValue);
	}
    if (!mFilterValues[sensorId].mStarted && (fabs(temperature - mFilterValues[sensorId].mValue) < 0.2))
    {
        //Return the filtered value next time
    	mFilterValues[sensorId].mStarted = true;
    	LOG(INFO) << "Filter stabilised (" << sensorId << ");";
    }

	for(auto listener: mListeners)
	{
		listener->sensorTemperature(sensorId, sendValue);
	}
}

void TemperatureFilter::sensorSetTemperatureUp(const std::string& sensorId)
{
	std::lock_guard<std::mutex> lock(mDataMutex);

	for(auto listener: mListeners)
	{
		listener->sensorSetTemperatureUp(sensorId);
	}
}

void TemperatureFilter::sensorSetTemperatureDown(const std::string& sensorId)
{
	std::lock_guard<std::mutex> lock(mDataMutex);

	for(auto listener: mListeners)
	{
		listener->sensorSetTemperatureDown(sensorId);
	}
}

void TemperatureFilter::registerTemperatureListener(LogicNs::TemperatureListenerIf* listener)
{
	std::lock_guard<std::mutex> lock(mDataMutex);
	mListeners.insert(listener);
}

void TemperatureFilter::unRegisterTemperatureListener(LogicNs::TemperatureListenerIf* listener)
{
	std::lock_guard<std::mutex> lock(mDataMutex);
	mListeners.erase(listener);
}

void TemperatureFilter::writeSetTemperature(const std::string& sensorId, double temperature)
{
	if (mSource)
	{
		mSource->writeSetTemperature(sensorId, temperature);
	}
}

void TemperatureFilter::writeSensorConfig(const std::string& sensorId, double calibration, const std::string& roomName)
{
	if (mSource)
	{
		mSource->writeSensorConfig(sensorId, calibration, roomName);
	}
}

void TemperatureFilter::writeHeaterOn(const std::string& sensorId)
{
	if (mSource)
	{
		mSource->writeHeaterOn(sensorId);
	}
}

void TemperatureFilter::writeHeaterOff(const std::string& sensorId)
{
	if (mSource)
	{
		mSource->writeHeaterOff(sensorId);
	}
}


} /* namespace LogicNs */
