/*
 * SystemClock.h
 *
 *  Created on: Aug 12, 2017
 *      Author: koen
 */

#ifndef SRC_LIB_SYSTEMCLOCK_H_
#define SRC_LIB_SYSTEMCLOCK_H_
#include "SystemClockIf.h"

namespace LogicNs
{

class SystemClock: public SystemClockIf
{
public:
	SystemClock();
	virtual ~SystemClock();

	LocalTime localTime();
};

} /* namespace LogicNs */

#endif /* SRC_LIB_SYSTEMCLOCK_H_ */
