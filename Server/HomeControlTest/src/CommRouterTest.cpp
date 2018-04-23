/*
 * CommRouterT.cpp
 *
 *  Created on: Jun 6, 2016
 *      Author: koen
 */

#include "Comm/CommServerIf.h"
#include "Comm/CommListenerIf.h"
#include "Comm/TemperatureSourceIf.h"
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

namespace
{
class HomeControlDalStub: public DalNs::HomeControlDalIf
{
public:
	HomeControlDalStub(): mRoomConfig(nullptr), mSensorCalibration() {};
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
	double getSensorCalibration(const std::string& sensorId) {return mSensorCalibration;};

	DalNs::RoomConfig* mRoomConfig;
	double mSensorCalibration;
};

class CommServerStub: public CommNs::CommServerIf
{
public:
	CommServerStub(): mLastName(), mLastObject(nullptr){};
	virtual ~CommServerStub()
	{
		if (mLastObject)
		{
			delete mLastObject;
		}
	};

	void registerCommListener(CommNs::CommListenerIf* listener) {};
	void unRegisterCommListener(CommNs::CommListenerIf* listener) {};
	// Server takes ownership
	void sendObject(const std::string name, CommNs::CommObjectIf* object)
	{
		if (mLastObject)
		{
			delete mLastObject;
		}
		mLastName = name;
		mLastObject = object;
	}
	std::string mLastName;
	CommNs::CommObjectIf* mLastObject;
};

class TemperatureSourceStub: public  CommNs::TemperatureSourceIf
{
public:
	TemperatureSourceStub() : mLastCalibration(), mLastRoomName(), mLastSensorId(), mLastTemperature()  {};
	virtual ~TemperatureSourceStub() {};

	void registerTemperatureListener(LogicNs::TemperatureListenerIf* listener) {};
	void unRegisterTemperatureListener(LogicNs::TemperatureListenerIf* listener) {};

	void writeSetTemperature(const std::string& sensorId, double temperature)
	{
		mLastSensorId = sensorId;
		mLastTemperature = temperature;
	};

	void writeSensorConfig(const std::string& sensorId, double calibration, const std::string& roomName)
	{
		mLastCalibration = calibration;
		mLastRoomName = roomName;
	};
	void writeTime(const std::string& name) {};
	double mLastCalibration;
	std::string mLastRoomName;
	std::string mLastSensorId;
	double mLastTemperature;
};
}

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
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	router->sensorTemperature("SensorId1", 23.6);
	// Temperature will not be send; no clients connected
	ASSERT_TRUE(commServerStub.mLastObject == nullptr);


	// A client is connected, the temperature will be send
	router->clientConnected("client1");
	router->sensorTemperature("SensorId1", 24.6);

	//Allow processing
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ASSERT_TRUE(commServerStub.mLastObject != nullptr);
	EXPECT_EQ(commServerStub.mLastName, "client1");


	if (CommNs::RoomTemperature* temperature = dynamic_cast<CommNs::RoomTemperature*>(commServerStub.mLastObject))
	{
		EXPECT_EQ(temperature->temperature(), 24.6);
	}

	delete router;
}

TEST(CommRouter, SendSensorConfig)
{
	HomeControlDalStub dalStub;
	CommServerStub commServerStub;
	TemperatureSourceStub tempSourceStub;
	// Will be deleted by CommRouter
	dalStub.mRoomConfig = new DalNs::RoomConfig;
	dalStub.mRoomConfig->RoomId = "RoomId";
	dalStub.mRoomConfig->RoomName = "RoomName";
	dalStub.mRoomConfig->mSensorIds.push_back("SensorId1");
	LogicNs::CommRouter* router = new LogicNs::CommRouter(&dalStub, &commServerStub, &tempSourceStub, nullptr);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	dalStub.mSensorCalibration = 1.5;

	router->sensorStarted("SensorId1");


	EXPECT_EQ(1.5, tempSourceStub.mLastCalibration);
	EXPECT_EQ("RoomName", tempSourceStub.mLastRoomName);


	delete router;
}

TEST(CommRouter, SetTempChanged)
{
	HomeControlDalStub dalStub;
	CommServerStub commServerStub;
	TemperatureSourceStub tempSourceStub;
	// Will be deleted by CommRouter
	dalStub.mRoomConfig = new DalNs::RoomConfig;
	dalStub.mRoomConfig->RoomId = "RoomId";
	dalStub.mRoomConfig->RoomName = "RoomName";
	dalStub.mRoomConfig->mSensorIds.push_back("SensorId1");
	LogicNs::CommRouter* router = new LogicNs::CommRouter(&dalStub, &commServerStub, &tempSourceStub, nullptr);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	dalStub.mSensorCalibration = 1.5;

	router->sensorStarted("SensorId1");

	router->setPointChanged("RoomId", 20);

	EXPECT_EQ(20, tempSourceStub.mLastTemperature);
	EXPECT_EQ("SensorId1", tempSourceStub.mLastSensorId);

	router->setPointChanged("RoomId", 22);
	EXPECT_EQ(22, tempSourceStub.mLastTemperature);


	delete router;
}
