/*
 * FullTest.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: koen
 */


#include "Comm/CommServerIf.h"
#include "Comm/CommListenerIf.h"
#include "Comm/TemperatureSensors.h"
#include "Comm/TemperatureSourceIf.h"
#include "Comm/DMCommIf.h"
#include "Comm/DMCommListenerIf.h"
#include "Comm/DMMessages.h"
#include "Comm/CommServerIf.h"
#include "CommObjects/CommObjectIf.h"
#include "CommObjects/RoomTemperature.h"
#include "Logic/CommRouter.h"
#include "Logic/TemperatureFilter.h"

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

class DMCommStub: public CommNs::DMCommIf
{
public:
	DMCommStub(): mAddressString(), mLastMessage(nullptr) {};
	virtual ~DMCommStub() {};

	void registerListener(CommNs::DMCommListenerIf* listener) {};
	void unRegisterListener(CommNs::DMCommListenerIf* listener) {};

	void init() {};
	std::string addressString()
	{
		return mAddressString;
	};

	void sendMessage(CommNs::DMMessageIf* message)
	{
		LOG(INFO) << "Async sendMessage: " << message->toString();
		mLastMessage = message;
	};
	CommNs::DMMessageIf* sendMessage(CommNs::DMMessageIf* message, int timeOutMilliseconds)
	{
		LOG(INFO) << "Sync sendMessage: " << message->toString();
		mLastMessage = message;
		return nullptr;
	};
	std::string mAddressString;
	CommNs::DMMessageIf* mLastMessage;

};

class CommServerStub: public CommNs::CommServerIf
{
public:
	CommServerStub() {};
	virtual ~CommServerStub() {};

	void registerCommListener(CommNs::CommListenerIf* listener) {};
	void unRegisterCommListener(CommNs::CommListenerIf* listener) {};
	// Server takes ownership
	void sendObject(const std::string name, CommNs::CommObjectIf* object)
	{
		LOG(INFO) << "sendObject: " << object->toString();
		delete object;
	};
};

TEST(FullTest, FullSetup)
{
	DMCommStub dmCommStub;
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(&dmCommStub);
	LogicNs::TemperatureFilter* filter = new LogicNs::TemperatureFilter(sensors, 0.2);
	CommServerStub* commServerStub = new CommServerStub();
	HomeControlDalStub dalStub;

	// Will be deleted by CommRouter
	dalStub.mRoomConfig = new DalNs::RoomConfig;
	dalStub.mRoomConfig->RoomId = "RoomId";
	dalStub.mRoomConfig->RoomName = "RoomName";
	dalStub.mRoomConfig->mSensorIds.push_back("AB");

	//DalNs::HomeControlDalIf* dal, CommNs::CommServerIf* server, CommNs::TemperatureSourceIf* sensors, HeaterListenerIf* heaterListener
	LogicNs::CommRouter* router = new LogicNs::CommRouter(&dalStub, commServerStub, filter, nullptr);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	CommNs::RxMessage* message = new CommNs::RxMessage(std::vector<uint8_t> {0, '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0,
																			'[', '1', ':', 'A', 'B', ']'});

	sensors->receiveMessage(message);

	//Calibration data
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ASSERT_NE(nullptr, dmCommStub.mLastMessage);
	delete dmCommStub.mLastMessage;
	dmCommStub.mLastMessage = nullptr;

	//Temp Up
	message = new CommNs::RxMessage(std::vector<uint8_t> {0, '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0,
																			'[', '6', ':', 'A', 'B', ']'});
	sensors->receiveMessage(message);

	//Set temperature
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ASSERT_NE(nullptr, dmCommStub.mLastMessage);

	delete dmCommStub.mLastMessage;
	dmCommStub.mLastMessage = nullptr;

	std::this_thread::sleep_for(std::chrono::seconds(30));
	delete router;
	delete commServerStub;
	delete filter;
	delete sensors;

}
