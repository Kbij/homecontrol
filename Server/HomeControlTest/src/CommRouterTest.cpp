/*
 * CommRouterT.cpp
 *
 *  Created on: Jun 6, 2016
 *      Author: koen
 */

#include "Comm/CommServerIf.h"
#include "Comm/CommListenerIf.h"
#include "CommObjects/CommObjectIf.h"
#include "CommObjects/RoomTemperature.h"
#include "Logic/CommRouter.h"
#include "DAL/HomeControlDalIf.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>
#include <atomic>

class HomeControlDalStub: public DalNs::HomeControlDalIf
{
public:
	HomeControlDalStub(): mRoomConfig(nullptr) {};
	virtual ~HomeControlDalStub() {};

	DalNs::RoomConfig* findRoomByRoomId(const std::string& roomId)
	{
		DalNs::RoomConfig* result = mRoomConfig;
		mRoomConfig = nullptr;
		return result;
	}
	DalNs::RoomConfig* findRoomBySensorId(const std::string& sensorId)
	{
		DalNs::RoomConfig* result = mRoomConfig;
		mRoomConfig = nullptr;
		return result;
	}
	DalNs::RoomConfig* mRoomConfig;
};

class CommServerStub: public CommNs::CommServerIf
{
public:
	CommServerStub(): mLastName(), mLastObject(nullptr){};
	virtual ~CommServerStub() {};

	void registerCommListener(CommNs::CommListenerIf* listener) {};
	void unRegisterCommListener(CommNs::CommListenerIf* listener) {};
	// Server takes ownership
	void sendObject(const std::string name, CommNs::CommObjectIf* object)
	{
		mLastName = name;
		mLastObject = object;
	}
	std::string mLastName;
	CommNs::CommObjectIf* mLastObject;
};


TEST(CommRouter, Constructor)
{
	LogicNs::CommRouter* router = new LogicNs::CommRouter(nullptr, nullptr, nullptr, nullptr);
	delete router;
}

TEST(CommRouter, UnknownSensor)
{
	HomeControlDalStub dalStub;
	LogicNs::CommRouter* router = new LogicNs::CommRouter(&dalStub, nullptr, nullptr, nullptr);

	router->sensorStarted("unknown SensorId");

	delete router;
}

TEST(CommRouter, KnownSensor)
{
	HomeControlDalStub dalStub;

	// Will be deleted by CommRouter
	dalStub.mRoomConfig = new DalNs::RoomConfig;
	dalStub.mRoomConfig->RoomId = "RoomId";
	dalStub.mRoomConfig->RoomName = "RoomName";
	LogicNs::CommRouter* router = new LogicNs::CommRouter(&dalStub, nullptr, nullptr, nullptr);

	router->sensorStarted("SensorId");

	router->sensorStarted("SensorId");

	delete router;
}

TEST(CommRouter, TwoSensorsSameRoom)
{
	HomeControlDalStub dalStub;

	// Will be deleted by CommRouter
	dalStub.mRoomConfig = new DalNs::RoomConfig;
	dalStub.mRoomConfig->RoomId = "RoomId";
	dalStub.mRoomConfig->RoomName = "RoomName";
	LogicNs::CommRouter* router = new LogicNs::CommRouter(&dalStub, nullptr, nullptr, nullptr);

	router->sensorStarted("SensorId1");

	// Do it a second time; both sensors point to
	dalStub.mRoomConfig = new DalNs::RoomConfig;
	dalStub.mRoomConfig->RoomId = "RoomId";
	dalStub.mRoomConfig->RoomName = "RoomName";
	router->sensorStarted("SensorId2");

	delete router;
}

TEST(CommRouter, SendTemperatureToCommServer)
{
	HomeControlDalStub dalStub;
	CommServerStub commServerStub;
	// Will be deleted by CommRouter
	dalStub.mRoomConfig = new DalNs::RoomConfig;
	dalStub.mRoomConfig->RoomId = "RoomId";
	dalStub.mRoomConfig->RoomName = "RoomName";
	dalStub.mRoomConfig->mSensorIds.push_back("SensorId1");
	LogicNs::CommRouter* router = new LogicNs::CommRouter(&dalStub, &commServerStub, nullptr, nullptr);

	router->sensorTemperature("SensorId1", 23.6);
	// Temperature will not be send; no clients connected
	ASSERT_TRUE(commServerStub.mLastObject == nullptr);


Blijft soms hangen, geen idee waarom (race conditie ?)

	// A client is connected, the temperature will be send
	router->clientConnected("client1");
	router->sensorTemperature("SensorId1", 24.6);

	ASSERT_TRUE(commServerStub.mLastObject != nullptr);
	EXPECT_EQ(commServerStub.mLastName, "client1");

	if (CommNs::RoomTemperature* temperature = dynamic_cast<CommNs::RoomTemperature*>(commServerStub.mLastObject))
	{
		EXPECT_EQ(temperature->temperature(), 24.6);
	}

	delete commServerStub.mLastObject;
	delete router;
}
