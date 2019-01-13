/*
 * HeaterControl.h
 *
 *  Created on: Jan 8, 2019
 *      Author: koen
 */

#ifndef LOGIC_HEATERCONTROL_H_
#define LOGIC_HEATERCONTROL_H_
#include "HeaterListenerIf.h"


namespace HwNs
{
class RelaisDriver;
}
namespace LogicNs {

class HeaterControl: public HeaterListenerIf
{
public:
	HeaterControl(HwNs::RelaisDriver* relais);
	virtual ~HeaterControl();

	void heaterOn(uint8_t heaterOutput);
	void heaterOff(uint8_t heaterOutput);
private:
	HwNs::RelaisDriver* mRelaisDriver;
};

} /* namespace LogicNs */

#endif /* LOGIC_HEATERCONTROL_H_ */
