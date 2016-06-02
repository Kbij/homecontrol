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

TemperatureFilter::TemperatureFilter(TemperatureListenerIf* listener, size_t sampleCount):
	mListener(listener),
	mSampleCount(sampleCount),
	mTemperatureHistory()
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
//	Filter* filter = getFilter(sensorId);
//	if (filter)
//	{
//		double filteredTemperature = filter->do_sample(temperature);
//		if (filter->hasEnoughSamples() && mListener)
//		{
//			mListener->sensorTemperature(sensorId, filteredTemperature);
//		}
//	}
	if (mListener)
	{
		mTemperatureHistory[sensorId].push_back(temperature);
		// Not enough samples -> pass thru
		if (mTemperatureHistory[sensorId].size() < mSampleCount)
		{
			mListener->sensorTemperature(sensorId, temperature);
		}
		else
		{
			double temperatureSum = std::accumulate(mTemperatureHistory[sensorId].begin(), mTemperatureHistory[sensorId].end(), 0.0);
			mListener->sensorTemperature(sensorId, temperatureSum/mSampleCount);
			mTemperatureHistory[sensorId].erase(mTemperatureHistory[sensorId].begin());
		}
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

//Filter* TemperatureFilter::getFilter(const std::string& sensorId)
//{
//	if (mFilters.find(sensorId) == mFilters.end())
//	{
//		// 30 Samples (all requency's are x1000)
//		// Sample frequency = 2kHz (1000 x 2Hz 30 seconds interval)
//		// Lowpass frequency = 100Hz
//		mFilters[sensorId] = new Filter(LPF, 51, 44.1, 2.0);
//	}
//	return mFilters[sensorId];
//}
} /* namespace LogicNs */
