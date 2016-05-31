/*
 * TemperatureListener.h
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */

#ifndef LOGIC_TEMPERATURELISTENERIF_H_
#define LOGIC_TEMPERATURELISTENERIF_H_
#include <string>

namespace LogicNs {

class TemperatureListenerIf
{
public:
	virtual ~TemperatureListenerIf() {};

	virtual void sensorStarted(const std::string& sensorId) = 0;
	virtual void sensorTemperature(const std::string& sensorId, float temperature) = 0;
	virtual void sensorSetTemperatureUp(const std::string& sensorId) = 0;
	virtual void sensorSetTemperatureDown(const std::string& sensorId) = 0;
};

} /* namespace CommNs */



#endif /* LOGIC_TEMPERATURELISTENERIF_H_ */
