/*
 * WatchDog.h
 *
 *  Created on: May 13, 2018
 *      Author: koen
 */

#ifndef HW_WATCHDOG_H_
#define HW_WATCHDOG_H_
#include <stdint.h>

namespace HwNs {
class I2CIf;


class WatchDog {
public:
	WatchDog(I2CIf* i2c, uint8_t driverAddress, uint8_t watchdogTimeSeconds);
	virtual ~WatchDog();

	void feed();
private:
	I2CIf* mI2C;
	uint8_t mDriverAddress;
	uint8_t mWatchDogTimeSeconds;

};

} /* namespace HwNs */

#endif /* HW_WATCHDOG_H_ */
