/*
 * WatchDog.cpp
 *
 *  Created on: May 13, 2018
 *      Author: koen
 */

#include "Hw/WatchDog.h"
#include "Hw/Crc8.h"
#include "Hw/I2CIf.h"
#include <vector>
#include <thread>

namespace
{
const uint8_t ENABLE_DISABLE_WATCHDOG_CMD = 'W';
const uint8_t FEED_WATCHDOG_CMD = 'w';
const uint8_t ENABLE_WATCHDOG = 1;
const uint8_t DISABLE_WATCHDOG = 1;
}

namespace HwNs
{

WatchDog::WatchDog(I2CIf* i2c, uint8_t driverAddress, uint8_t watchdogTimeSeconds):
	mI2C(i2c),
	mDriverAddress(driverAddress),
	mWatchDogTimeSeconds(watchdogTimeSeconds)
{
	//Feed first
	feed();

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	//Then enable
	std::vector<uint8_t> enableWd;
	enableWd.push_back(ENABLE_DISABLE_WATCHDOG_CMD);
	enableWd.push_back(ENABLE_WATCHDOG);
	enableWd.push_back(Crc8(enableWd));
	if (mI2C)
	{
		mI2C->writeData(mDriverAddress, enableWd);
	}
}

WatchDog::~WatchDog()
{
	std::vector<uint8_t> data;
	data.push_back(ENABLE_DISABLE_WATCHDOG_CMD);
	data.push_back(DISABLE_WATCHDOG);
	data.push_back(Crc8(data));
	if (mI2C)
	{
		mI2C->writeData(mDriverAddress, data);
	}
}

void WatchDog::feed()
{
	std::vector<uint8_t> data;
	data.push_back(FEED_WATCHDOG_CMD);
	data.push_back(mWatchDogTimeSeconds);
	data.push_back(Crc8(data));
	if (mI2C)
	{
		mI2C->writeData(mDriverAddress, data);
	}
}
} /* namespace CommNs */
