/*
 * RelaisDriverTest.cpp
 *
 *  Created on: May 13, 2018
 *      Author: koen
 */



#include "Hw/Crc8.h"
#include "Hw/I2CIf.h"
#include "Hw/RelaisDriver.h"
#include <gtest/gtest.h>

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
		mData = data;
		return true;
	}
	uint8_t mAdress;
	std::vector<uint8_t> mData;
};
}
TEST(RelaisDriver, Crc1)
{
	std::vector<uint8_t> data;
	data.push_back(54);
	data.push_back(2);
	EXPECT_EQ((uint8_t) 59, Crc8(data));
}

TEST(RelaisDriver, Crc2)
{
	std::vector<uint8_t> data;
	data.push_back(55);
	data.push_back(2);
	EXPECT_EQ((uint8_t) 255, Crc8(data));
}

TEST(RelaisDriver, Constructor)
{
	HwNs::RelaisDriver* driver = new HwNs::RelaisDriver(nullptr, 0, 0);
	delete driver;
}

TEST(RelaisDriver, DriveRelaisOn)
{
	I2CStub* i2c = new I2CStub;
	HwNs::RelaisDriver* driver = new HwNs::RelaisDriver(i2c, 8, 2);

	driver->relaisOn('0');
	EXPECT_EQ((uint8_t) 8, i2c->mAdress);
	EXPECT_EQ(std::vector<uint8_t>({'0', 2, 145}), i2c->mData);

	driver->relaisOn('1');
	EXPECT_EQ((uint8_t) 8, i2c->mAdress);
	EXPECT_EQ(std::vector<uint8_t>({'1', 2, 85}), i2c->mData);

	driver->relaisOn('7');
	EXPECT_EQ((uint8_t) 8, i2c->mAdress);
	EXPECT_EQ(std::vector<uint8_t>({'7', 2, 255}), i2c->mData);

	delete driver;
	delete i2c;
}

TEST(RelaisDriver, DriveRelaisOff)
{
	I2CStub* i2c = new I2CStub;
	HwNs::RelaisDriver* driver = new HwNs::RelaisDriver(i2c, 8, 2);

	driver->relaisOn('0');
	EXPECT_EQ((uint8_t) 8, i2c->mAdress);
	EXPECT_EQ(std::vector<uint8_t>({'0', 2, 145}), i2c->mData);

	driver->relaisOff('0');
	EXPECT_EQ((uint8_t) 8, i2c->mAdress);
	EXPECT_EQ(std::vector<uint8_t>({'0', 0, 45}), i2c->mData);

	delete driver;
	delete i2c;
}
