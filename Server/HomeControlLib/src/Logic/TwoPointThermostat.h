/*
 * TwoPointThermostat.h
 *
 *  Created on: Jan 8, 2019
 *      Author: koen
 */

#ifndef LOGIC_TWOPOINTTHERMOSTAT_H_
#define LOGIC_TWOPOINTTHERMOSTAT_H_
#include "ThermostatIf.h"
#include <string>
#include <stdint.h>

namespace LogicNs {
class ThermostatListenerIf;

//RoomListenerIf is only used for signallinig the heater on/off
class TwoPointThermostat: public ThermostatIf
{
public:
	TwoPointThermostat(double hysteresis);
	virtual ~TwoPointThermostat();

	//ThermostatIf
	void registerListener(ThermostatListenerIf* listener);
	void unRegisterListener();
	void temperatureChanged(time_t timePoint, double temperature);
	void setPointChanged(time_t timePoint, double setTemperature);

private:
	const double mHysteresis;
	ThermostatListenerIf* mListener;
	double mSetPoint;
	bool mHeaterOn;

};

} /* namespace LogicNs */

#endif /* LOGIC_TWOPOINTTHERMOSTAT_H_ */
