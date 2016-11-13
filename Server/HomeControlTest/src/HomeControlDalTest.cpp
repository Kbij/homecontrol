/*
 * HomeControlDalTest.cpp
 *
 *  Created on: Jun 10, 2016
 *      Author: koen
 */

#include "DAL/HomeControlDal.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>
#include <stdint.h>


TEST(HomeControlDal, Constructor)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal;

	delete dal;
}

TEST(HomeControlDal, FindSensor)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal;
	DalNs::RoomConfig* roomConfig = dal->findRoomBySensorId("sensor1");

	EXPECT_NE(nullptr, roomConfig);
	EXPECT_EQ("Kamer1", roomConfig->RoomName);
	EXPECT_EQ("K1", roomConfig->RoomId);
	std::vector<std::string> sensorIds({"sensor1", "sensor1.1"});
	EXPECT_EQ(sensorIds, roomConfig->mSensorIds);
	delete roomConfig;

	delete dal;
}

TEST(HomeControlDal, FindSensorNoResult)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal;
	DalNs::RoomConfig* roomConfig = dal->findRoomBySensorId("nonexistentsensor");

	EXPECT_EQ(nullptr, roomConfig);

	delete dal;
}

TEST(HomeControlDal, FindRoom)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal;
	DalNs::RoomConfig* roomConfig = dal->findRoomByRoomId("K1");

	EXPECT_NE(nullptr, roomConfig);
	EXPECT_EQ("Kamer1", roomConfig->RoomName);
	EXPECT_EQ("K1", roomConfig->RoomId);
	std::vector<std::string> sensorIds({"sensor1", "sensor1.1"});
	EXPECT_EQ(sensorIds, roomConfig->mSensorIds);
	delete roomConfig;

	delete dal;
}

TEST(HomeControlDal, NonExistentRoom)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal;
	DalNs::RoomConfig* roomConfig = dal->findRoomByRoomId("nonexistentroom");

	EXPECT_EQ(nullptr, roomConfig);

	delete dal;
}
TEST(HomeControlDal, FindCalibration)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal;

	EXPECT_EQ(1, dal->getSensorCalibration("sensor1"));
	EXPECT_EQ(1.1, dal->getSensorCalibration("sensor1.1"));

	delete dal;
}
