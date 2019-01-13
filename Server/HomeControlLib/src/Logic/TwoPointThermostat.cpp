/*
 * TwoPointThermostat.cpp
 *
 *  Created on: Jan 8, 2019
 *      Author: koen
 */

#include "Logic/TwoPointThermostat.h"
#include "Logic/ThermostatListenerIf.h"

namespace LogicNs {

TwoPointThermostat::TwoPointThermostat(double hysteresis):
	mHysteresis(hysteresis),
	mListener(),
	mSetPoint(),
	mHeaterOn()
{
}

TwoPointThermostat::~TwoPointThermostat()
{
}

void TwoPointThermostat::registerListener(ThermostatListenerIf* listener)
{
	mListener = listener;
}

void TwoPointThermostat::unRegisterListener()
{
	mListener = nullptr;
}

void TwoPointThermostat::temperatureChanged(time_t timePoint, double temperature)
{
	double hysteresisTop = mSetPoint + (mHysteresis/2.0);
	double hysteresisBotom = mSetPoint - (mHysteresis/2.0);
	//bool inHysteresis = true;
	if (temperature > hysteresisTop)
	{
		mHeaterOn = false;
		//if (mListener) mListener->heaterOff();
	}
	if (temperature < hysteresisBotom)
	{
		mHeaterOn = true;
		//if (mListener) mListener->heaterOn();
	}
	if (mHeaterOn)
	{
		if (mListener) mListener->heaterOn();
	}
	else
	{
		if (mListener) mListener->heaterOff();
	}
}

void TwoPointThermostat::setPointChanged(time_t timePoint, double setTemperature)
{
	mSetPoint = setTemperature;
}

} /* namespace LogicNs */
