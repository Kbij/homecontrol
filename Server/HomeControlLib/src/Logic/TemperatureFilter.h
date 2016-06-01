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

namespace LogicNs {

class TemperatureFilter: public TemperatureListenerIf
{
public:
	TemperatureFilter(TemperatureListenerIf* listener);
	virtual ~TemperatureFilter();

	void sensorStarted(const std::string& sensorId);
	void sensorTemperature(const std::string& sensorId, double temperature);
	void sensorSetTemperatureUp(const std::string& sensorId);
	void sensorSetTemperatureDown(const std::string& sensorId);
private:
	TemperatureListenerIf* mListener;
	std::map<std::string, Filter*> mFilters;
	Filter* getFilter(const std::string& sensorId);
};

} /* namespace LogicNs */

#endif /* LOGIC_TEMPERATUREFILTER_H_ */
