/*
 * RelaisDriver.cpp
 *
 *  Created on: May 12, 2018
 *      Author: koen
 */

#include <Hw/RelaisDriver.h>
#include <Hw/Crc8.h>
#include <Hw/I2CIf.h>
#include <vector>
#include <glog/logging.h>

namespace HwNs
{

RelaisDriver::RelaisDriver(I2CIf* i2c, uint8_t driverAddress, uint8_t waitTimeSeconds):
	mI2C(i2c),
	mDriverAddress(driverAddress),
	mWaitTimeSeconds(waitTimeSeconds)
{
}

RelaisDriver::~RelaisDriver()
{
}

void RelaisDriver::relaisOn(char relaisId)
{
	if (checkRelaisId(relaisId))
	{
		std::vector<uint8_t> data;
		data.push_back(relaisId);
		data.push_back(mWaitTimeSeconds);
		data.push_back(Crc8(data));
		if (mI2C)
		{
			mI2C->writeData(mDriverAddress, data);
		}
	}
}

void RelaisDriver::relaisOff(char relaisId)
{
	if (checkRelaisId(relaisId))
	{
		std::vector<uint8_t> data;
		data.push_back(relaisId);
		data.push_back(0);
		data.push_back(Crc8(data));
		if (mI2C)
		{
			mI2C->writeData(mDriverAddress, data);
		}
	}
}

bool RelaisDriver::checkRelaisId(char relaisId)
{
	switch(relaisId)
	{
		case '0': return true;
		case '1': return true;
		case '2': return true;
		case '3': return true;
		case '4': return true;
		case '5': return true;
		case '6': return true;
		case '7': return true;
	}

	LOG(ERROR) << "Invalid relais ID: " << relaisId;

	return false;
}

} /* namespace HwNs */
