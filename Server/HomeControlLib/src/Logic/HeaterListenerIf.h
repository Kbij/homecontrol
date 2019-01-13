/*
 * HeaterListenerIf.h
 *
 *  Created on: Aug 11, 2016
 *      Author: koen
 */

#ifndef LOGIC_HEATERLISTENERIF_H_
#define LOGIC_HEATERLISTENERIF_H_
#include <string>
#include <stdint.h>

namespace LogicNs {


class HeaterListenerIf
{
public:
	virtual ~HeaterListenerIf() {};

	virtual void heaterOn(uint8_t heaterOutput) = 0;
	virtual void heaterOff(uint8_t heaterOutput) = 0;
};

} /* namespace LogicNs */



#endif /* LOGIC_HEATERLISTENERIF_H_ */
