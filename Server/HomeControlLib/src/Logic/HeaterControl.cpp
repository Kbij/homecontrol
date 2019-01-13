/*
 * HeaterControl.cpp
 *
 *  Created on: Jan 8, 2019
 *      Author: koen
 */

#include "Logic/HeaterControl.h"
#include "Hw/RelaisDriver.h"

namespace LogicNs {

HeaterControl::HeaterControl(HwNs::RelaisDriver* relais):
	mRelaisDriver(relais)
{
}

HeaterControl::~HeaterControl()
{
}

char heaterIdToRelaisId(uint8_t heaterOutput)
{
	switch(heaterOutput)
	{
		case 0: return '0';
		case 1: return '1';
		case 2: return '2';
		case 3: return '3';
		case 4: return '4';
		case 5: return '5';
		case 6: return '6';
		case 7: return '7';
		case 8: return '8';
		default: return '0';
	}
}

void HeaterControl::heaterOn(uint8_t heaterOutput)
{
	if (mRelaisDriver)
	{
		mRelaisDriver->relaisOn(heaterIdToRelaisId(heaterOutput));
	}
}

void HeaterControl::heaterOff(uint8_t heaterOutput)
{
	if (mRelaisDriver)
	{
		mRelaisDriver->relaisOff(heaterIdToRelaisId(heaterOutput));
	}
}
} /* namespace LogicNs */