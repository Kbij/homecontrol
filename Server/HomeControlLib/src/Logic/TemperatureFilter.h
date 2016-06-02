/*
 * TemperatureFilter.h
 *
 *  Created on: Jun 1, 2016
 *      Author: koen
 */

#ifndef LOGIC_TEMPERATUREFILTER_H_
#define LOGIC_TEMPERATUREFILTER_H_
#include "TemperatureListenerIf.h"
#include "Filter.h"
#include <map>
#include <vector>

namespace LogicNs {

class TemperatureFilter: public TemperatureListenerIf
{
public:
	TemperatureFilter(TemperatureListenerIf* listener, size_t sampleCount);
	virtual ~TemperatureFilter();

	void sensorStarted(const std::string& sensorId);
	void sensorTemperature(const std::string& sensorId, double temperature);
	void sensorSetTemperatureUp(const std::string& sensorId);
	void sensorSetTemperatureDown(const std::string& sensorId);
private:
	TemperatureListenerIf* mListener;
	const size_t mSampleCount;
	std::map<std::string, std::vector<double>> mTemperatureHistory;
//	Filter* getFilter(const std::string& sensorId);
};

} /* namespace LogicNs */

#endif /* LOGIC_TEMPERATUREFILTER_H_ */
