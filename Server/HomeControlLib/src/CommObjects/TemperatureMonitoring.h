/*
 * TemperatureMonitoring.h
 *
 *  Created on: Jun 12, 2016
 *      Author: koen
 */

#ifndef COMMOBJECTS_TEMPERATUREMONITORING_H_
#define COMMOBJECTS_TEMPERATUREMONITORING_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>

namespace CommNs
{

class TemperatureMonitoring: public CommObjectIf
{
public:
	TemperatureMonitoring();
	TemperatureMonitoring(const std::string& json);
	virtual ~TemperatureMonitoring();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;

	bool monitoringEnabled() const {return mMonitoringEnabled;};
private:
	bool mMonitoringEnabled;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_TEMPERATUREMONITORING_H_ */
