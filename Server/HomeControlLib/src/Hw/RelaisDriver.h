/*
 * RelaisDriver.h
 *
 *  Created on: May 12, 2018
 *      Author: koen
 */

#ifndef HW_RELAISDRIVER_H_
#define HW_RELAISDRIVER_H_
#include <stdint.h>

namespace HwNs {
class I2CIf;


class RelaisDriver
{
public:
	RelaisDriver(I2CIf* i2c, uint8_t driverAddress, uint8_t waitTimeSeconds);
	virtual ~RelaisDriver();

	/**
	 * Switch the relais on, based on the id ('0' .. '7')
	 */
	void relaisOn(char relaisId);

	/**
	 * Switch the relais off, based on the id ('0' .. '7')
	 */
	void relaisOff(char relaisId);
private:
	bool checkRelaisId(char relaisId);
	I2CIf* mI2C;
	uint8_t mDriverAddress;
	uint8_t mWaitTimeSeconds;
};

} /* namespace HwNs */

#endif /* HW_RELAISDRIVER_H_ */
