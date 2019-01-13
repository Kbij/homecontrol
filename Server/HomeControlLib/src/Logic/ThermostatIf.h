/*
 * ThermostatIf.h
 *
 *  Created on: Jan 8, 2019
 *      Author: koen
 */

#ifndef LOGIC_THERMOSTATIF_H_
#define LOGIC_THERMOSTATIF_H_
#include <ctime>

namespace LogicNs {
class ThermostatListenerIf;

class ThermostatIf
{
public:
	virtual ~ThermostatIf() {};

	virtual void registerListener(ThermostatListenerIf* listener) = 0;
	virtual void unRegisterListener() = 0;

	virtual void temperatureChanged(time_t timePoint, double temperature) = 0;
	virtual void setPointChanged(time_t timePoint, double setTemperature) = 0;
};

} /* namespace LogicNs */



#endif /* LOGIC_THERMOSTATIF_H_ */
