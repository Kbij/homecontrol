/*
 * ThermostatListenerIf.h
 *
 *  Created on: Jan 13, 2019
 *      Author: koen
 */

#ifndef LOGIC_THERMOSTATLISTENERIF_H_
#define LOGIC_THERMOSTATLISTENERIF_H_

namespace LogicNs {

class ThermostatListenerIf
{
public:
	virtual void heaterOn() = 0;
	virtual void heaterOff() = 0;
};

} /* namespace LogicNs */



#endif /* LOGIC_THERMOSTATLISTENERIF_H_ */
