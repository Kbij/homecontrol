/*
 * TemperatureFilter.cpp
 *
 *  Created on: Jun 1, 2016
 *      Author: koen
 */

#include <Logic/TemperatureFilter.h>

namespace LogicNs {

TemperatureFilter::TemperatureFilter(TemperatureListenerIf* listener):
	mListener(listener),
	mFilters()
{
}

TemperatureFilter::~TemperatureFilter()
{
}

void TemperatureFilter::sensorStarted(const std::string& sensorId)
{
	if (mListener) mListener->sensorStarted(sensorId);
}

void TemperatureFilter::sensorTemperature(const std::string& sensorId, double temperature)
{
	Filter* filter = getFilter(sensorId);
	if (filter)
	{
		double filteredTemperature = filter->do_sample(temperature);
		if (mListener) mListener->sensorTemperature(sensorId, filteredTemperature);
	}
}

void TemperatureFilter::sensorSetTemperatureUp(const std::string& sensorId)
{
	if (mListener) mListener->sensorSetTemperatureUp(sensorId);
}

void TemperatureFilter::sensorSetTemperatureDown(const std::string& sensorId)
{
	if (mListener) mListener->sensorSetTemperatureDown(sensorId);
}

Filter* TemperatureFilter::getFilter(const std::string& sensorId)
{
	if (mFilters.find(sensorId) == mFilters.end())
	{
		// 30 Samples (all requency's are x1000)
		// Sample frequency = 2kHz (1000 x 2Hz 30 seconds interval)
		// Lowpass frequency = 100Hz
		mFilters[sensorId] = new Filter(LPF, 30, 2000, 50);
	}
	return mFilters[sensorId];
}
} /* namespace LogicNs */
