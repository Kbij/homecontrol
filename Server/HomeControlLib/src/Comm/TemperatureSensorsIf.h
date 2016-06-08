/*
 * TemperatureSensorsIf.h
 *
 *  Created on: Jun 7, 2016
 *      Author: koen
 */

#ifndef COMM_TEMPERATURESENSORSIF_H_
#define COMM_TEMPERATURESENSORSIF_H_
#include <string>

namespace LogicNs
{
class TemperatureListenerIf;
}

namespace CommNs {

class TemperatureSensorsIf
{
public:
	virtual ~TemperatureSensorsIf() {};

	virtual void registerTemperatureListener(LogicNs::TemperatureListenerIf* listener) = 0;
	virtual void unRegisterTemperatureListener(LogicNs::TemperatureListenerIf* listener) = 0;

	virtual void writeSetTemperature(const std::string& sensorId, double temperature) = 0;
};

} /* namespace CommNs */


#endif /* COMM_TEMPERATURESENSORSIF_H_ */
