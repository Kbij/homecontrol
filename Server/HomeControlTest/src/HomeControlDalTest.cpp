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

namespace
{
const std::string SERVER = "tcp://localhost:3306";
const std::string USER = "hc";
const std::string PWD = "bugs bunny";
const int PORT = 3306;
const std::string DB = "hc";
}

TEST(HomeControlDal, Constructor)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal(SERVER, PORT, DB, USER, PWD);

	delete dal;
}

TEST(HomeControlDal, FindSensor)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal(SERVER, PORT, DB, USER, PWD);
	DalNs::RoomConfig* roomConfig = dal->findRoomBySensorId("sensor1");

	EXPECT_NE(nullptr, roomConfig);
	EXPECT_EQ("Kamer1", roomConfig->RoomName);
	EXPECT_EQ("K1", roomConfig->RoomId);
	EXPECT_EQ(2, roomConfig->HeaterOutput);
	std::vector<std::string> sensorIds({"sensor1", "sensor1.1"});
	EXPECT_EQ(sensorIds, roomConfig->SensorIds);
	delete roomConfig;

	delete dal;
}

TEST(HomeControlDal, FindSensorNoResult)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal(SERVER, PORT, DB, USER, PWD);
	DalNs::RoomConfig* roomConfig = dal->findRoomBySensorId("nonexistentsensor");

	EXPECT_EQ(nullptr, roomConfig);

	delete dal;
}

TEST(HomeControlDal, FindRoom)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal(SERVER, PORT, DB, USER, PWD);
	DalNs::RoomConfig* roomConfig = dal->findRoomByRoomId("K1");

	EXPECT_NE(nullptr, roomConfig);
	EXPECT_EQ("Kamer1", roomConfig->RoomName);
	EXPECT_EQ("K1", roomConfig->RoomId);
	EXPECT_EQ(2, roomConfig->HeaterOutput);
	std::vector<std::string> sensorIds({"sensor1", "sensor1.1"});
	EXPECT_EQ(sensorIds, roomConfig->SensorIds);
	delete roomConfig;

	delete dal;
}

TEST(HomeControlDal, NonExistentRoom)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal(SERVER, PORT, DB, USER, PWD);
	DalNs::RoomConfig* roomConfig = dal->findRoomByRoomId("nonexistentroom");

	EXPECT_EQ(nullptr, roomConfig);

	delete dal;
}
TEST(HomeControlDal, FindCalibration)
{
	DalNs::HomeControlDal* dal = new DalNs::HomeControlDal(SERVER, PORT, DB, USER, PWD);

	EXPECT_EQ(1, dal->getSensorCalibration("sensor1"));
	EXPECT_EQ(1.1, dal->getSensorCalibration("sensor1.1"));

	delete dal;
}
