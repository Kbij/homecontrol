/*
 * TemperatureFilter.cpp
 *
 *  Created on: Jun 1, 2016
 *      Author: koen
 */

#include <Logic/TemperatureFilter.h>
#include <numeric>
namespace
{
const int SAMPLE_COUNT = 30;
}

namespace LogicNs {

TemperatureFilter::TemperatureFilter(CommNs::TemperatureSourceIf* source, int k):
	mSource(source),
	mDataMutex(),
	mListeners(),
	mK(k),
	mSampleCount(0),
	mFilterSum(0)
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

	mFilterSum = mFilterSum - (mFilterSum/mK) + temperature;
	double filteredTemperature = mFilterSum/mK;
	filteredTemperature = round(filteredTemperature * 100)/100;
	for(auto listener: mListeners)
	{
		if (mSampleCount > (mK * 5))
		{
			listener->sensorTemperature(sensorId, filteredTemperature);
		}
		else
		{
			++mSampleCount;
			listener->sensorTemperature(sensorId, temperature);
		}
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
} /* namespace LogicNs */
