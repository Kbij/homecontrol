/*
 * WatchDogTest.cpp
 *
 *  Created on: May 13, 2018
 *      Author: koen
 */
#include "Hw/Crc8.h"
#include "Hw/I2CIf.h"
#include "Hw/WatchDog.h"
#include <gtest/gtest.h>
#include <thread>

namespace
{
class I2CStub: public HwNs::I2CIf
{
public:
	I2CStub(): mAdress(), mData() {};
	virtual ~I2CStub() {};
	bool writeData(uint8_t address, const std::vector<uint8_t>& data)
	{
		mAdress = address;
		mData.push_back(data);
		return true;
	}
	uint8_t mAdress;
	std::vector<std::vector<uint8_t>> mData;
};
}

TEST(WatchDogTest, Constructor)
{
	HwNs::WatchDog* watchDog = new HwNs::WatchDog(nullptr, 0, 0);
	delete watchDog;
}

TEST(WatchDogTest, EnableDisableWd)
{
	I2CStub* i2c = new I2CStub;

	HwNs::WatchDog* watchDog = new HwNs::WatchDog(i2c, 8, 30);

	//Needs to send 2 packets
	std::this_thread::sleep_for(std::chrono::milliseconds(700));

	EXPECT_EQ((uint8_t) 8, i2c->mAdress);
	ASSERT_EQ((size_t) 2, i2c->mData.size());

	//Feed
	EXPECT_EQ(std::vector<uint8_t>({'w', 30, 90}), i2c->mData[0]);

	//Enable
	EXPECT_EQ(std::vector<uint8_t>({'W', 1, 71}), i2c->mData[1]);

	i2c->mData.clear();
	delete watchDog;

	//Should be disabled
	EXPECT_EQ(std::vector<uint8_t>({'W', 0, 71}), i2c->mData[1]);

	delete i2c;
}

TEST(WatchDogTest, Feed)
{
	I2CStub* i2c = new I2CStub;

	HwNs::WatchDog* watchDog = new HwNs::WatchDog(i2c, 8, 30);
	std::this_thread::sleep_for(std::chrono::milliseconds(700));
	i2c->mData.clear();

	watchDog->feed();

	ASSERT_EQ((size_t) 1, i2c->mData.size());
	EXPECT_EQ(std::vector<uint8_t>({'w', 30, 90}), i2c->mData[0]);

	delete watchDog;
	delete i2c;
}

