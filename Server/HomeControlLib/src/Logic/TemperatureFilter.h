/*
 * TemperatureFilter.h
 *
 *  Created on: Jun 1, 2016
 *      Author: koen
 */

#ifndef LOGIC_TEMPERATUREFILTER_H_
#define LOGIC_TEMPERATUREFILTER_H_
#include <Comm/TemperatureSourceIf.h>
#include "TemperatureListenerIf.h"
#include "Filter.h"
#include <map>
#include <vector>
#include <set>
#include <mutex>

namespace LogicNs {

class TemperatureFilter: public TemperatureListenerIf, public CommNs::TemperatureSourceIf
{
public:
	TemperatureFilter(CommNs::TemperatureSourceIf* source, int k);
	virtual ~TemperatureFilter();

	void sensorStarted(const std::string& sensorId);
	void sensorTemperature(const std::string& sensorId, double temperature);
	void sensorSetTemperatureUp(const std::string& sensorId);
	void sensorSetTemperatureDown(const std::string& sensorId);

	void registerTemperatureListener(LogicNs::TemperatureListenerIf* listener);
	void unRegisterTemperatureListener(LogicNs::TemperatureListenerIf* listener);

	void writeSetTemperature(const std::string& sensorId, double temperature);
private:
	CommNs::TemperatureSourceIf* mSource;
	std::mutex mDataMutex;
	std::set<TemperatureListenerIf*> mListeners;
	const int mK;
	int mSampleCount;
	float mFilterSum;
};

} /* namespace LogicNs */

#endif /* LOGIC_TEMPERATUREFILTER_H_ */
