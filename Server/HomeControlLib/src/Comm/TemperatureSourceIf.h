/*
 * TemperatureSensorsIf.h
 *
 *  Created on: Jun 7, 2016
 *      Author: koen
 */

#ifndef COMM_TEMPERATURESOURCEIF_H_
#define COMM_TEMPERATURESOURCEIF_H_
#include <string>
#include "TimeListenerIf.h"

namespace LogicNs
{
class TemperatureListenerIf;
}

namespace CommNs {

class TemperatureSourceIf: public TimeListenerIf
{
public:
	virtual ~TemperatureSourceIf() {};

	virtual void registerTemperatureListener(LogicNs::TemperatureListenerIf* listener) = 0;
	virtual void unRegisterTemperatureListener(LogicNs::TemperatureListenerIf* listener) = 0;

	virtual void writeSetTemperature(const std::string& sensorId, double temperature) = 0;
	virtual void writeSensorConfig(const std::string& sensorId, double calibration, const std::string& roomName) = 0;
	virtual void writeHeaterOn(const std::string& sensorId) = 0;
	virtual void writeHeaterOff(const std::string& sensorId) = 0;
};

} /* namespace CommNs */


#endif /* COMM_TEMPERATURESOURCEIF_H_ */
