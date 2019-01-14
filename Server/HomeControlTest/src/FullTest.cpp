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
#include <queue>
#include <queue>

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
	int locationInterval(const std::string& clientId) {return 0;};
	void writeHeaterOn(const std::string& roomId) {};
	void writeHeaterOff(const std::string& roomId) {};
	DalNs::RoomConfig* mRoomConfig;
	double mSensorCalibration;
};

class DMCommStub: public CommNs::DMCommIf
{
public:
	DMCommStub(): mAddressString(), mDataMutex(), mMessages() {};
	virtual ~DMCommStub()
	{
		while (mMessages.size() > 0)
		{
			CommNs::DMMessageIf* message = mMessages.front();
			delete message;
			mMessages.pop();
		}
	};

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
		mMessages.push(message);
	};

	CommNs::DMMessageIf* sendMessage(CommNs::DMMessageIf* message, int timeOutMilliseconds)
	{
		LOG(INFO) << "Sync sendMessage: " << message->toString();
		mMessages.push(message);
		return nullptr;
	};

	size_t queueSize()
	{
		std::lock_guard<std::mutex> lg(mDataMutex);
		return mMessages.size();
	}

	CommNs::DMMessageIf* popMessage()
	{
		CommNs::DMMessageIf* result = nullptr;
		std::lock_guard<std::mutex> lg(mDataMutex);
		if (mMessages.size() > 0)
		{
			result = mMessages.front();
			mMessages.pop();
		}
		return result;
	}
	std::string mAddressString;

private:
	std::mutex mDataMutex;
	std::queue<CommNs::DMMessageIf*> mMessages;
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
}

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
	dalStub.mRoomConfig->SensorIds.push_back("AB");

	//DalNs::HomeControlDalIf* dal, CommNs::CommServerIf* server, CommNs::TemperatureSourceIf* sensors, HeaterListenerIf* heaterListener
	LogicNs::CommRouter* router = new LogicNs::CommRouter(&dalStub, commServerStub, filter, nullptr);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	CommNs::RxMessage* message = new CommNs::RxMessage(std::vector<uint8_t> {0, '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0,
																			'[', '1', ':', 'A', 'B', ']'});

	sensors->receiveMessage(message);
	delete message;

	//Calibration data
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ASSERT_EQ(1, dmCommStub.queueSize());
	CommNs::DMMessageIf* receivedMessage = dmCommStub.popMessage();
	ASSERT_NE(nullptr, receivedMessage);
	CommNs::TxMessage* txMessage = dynamic_cast<CommNs::TxMessage*> (receivedMessage);
	ASSERT_NE(nullptr, txMessage);
	//Calibration data
	EXPECT_EQ("[8:0.00:RoomName]", txMessage->txString());

	delete txMessage;


	delete router;
	delete commServerStub;
	delete filter;
	delete sensors;
}

TEST(FullTest, SingleTempDown)
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
	dalStub.mRoomConfig->SensorIds.push_back("AB");

	//DalNs::HomeControlDalIf* dal, CommNs::CommServerIf* server, CommNs::TemperatureSourceIf* sensors, HeaterListenerIf* heaterListener
	LogicNs::CommRouter* router = new LogicNs::CommRouter(&dalStub, commServerStub, filter, nullptr);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	CommNs::RxMessage* message = new CommNs::RxMessage(std::vector<uint8_t> {0, '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0,
																			'[', '1', ':', 'A', 'B', ']'});

	sensors->receiveMessage(message);
	delete message;

	//Calibration data
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ASSERT_EQ(1, dmCommStub.queueSize());
	CommNs::DMMessageIf* receivedMessage = dmCommStub.popMessage();
	ASSERT_NE(nullptr, receivedMessage);
	CommNs::TxMessage* txMessage = dynamic_cast<CommNs::TxMessage*> (receivedMessage);
	ASSERT_NE(nullptr, txMessage);
	//Calibration data
	EXPECT_EQ("[8:0.00:RoomName]", txMessage->txString());

	delete txMessage;

	//Temp Down
	message = new CommNs::RxMessage(std::vector<uint8_t> {0, '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0,
																			'[', '7', ':', 'A', 'B', ']'});
	sensors->receiveMessage(message);
	delete message;

	//Set temperature
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ASSERT_EQ(1, dmCommStub.queueSize());
	receivedMessage = dmCommStub.popMessage();
	ASSERT_NE(nullptr, receivedMessage);
	txMessage = dynamic_cast<CommNs::TxMessage*> (receivedMessage);
	ASSERT_NE(nullptr, txMessage);
	EXPECT_EQ("[5:16.2]", txMessage->txString());
	delete txMessage;

	const int MESSAGES = 40;

	for (int i = 0; i < MESSAGES; ++i)
	{
		message = new CommNs::RxMessage(std::vector<uint8_t> {0, '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0,
																				'[', '7', ':', 'A', 'B', ']'});
		sensors->receiveMessage(message);
		delete message;
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
	//Will have received 20 set temperature's
	EXPECT_EQ(MESSAGES, dmCommStub.queueSize());

	delete router;
	delete commServerStub;
	delete filter;
	delete sensors;
}

TEST(FullTest, TempDownToLimit)
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
	dalStub.mRoomConfig->SensorIds.push_back("AB");

	//DalNs::HomeControlDalIf* dal, CommNs::CommServerIf* server, CommNs::TemperatureSourceIf* sensors, HeaterListenerIf* heaterListener
	LogicNs::CommRouter* router = new LogicNs::CommRouter(&dalStub, commServerStub, filter, nullptr);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	CommNs::RxMessage* message = new CommNs::RxMessage(std::vector<uint8_t> {0, '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0,
																			'[', '1', ':', 'A', 'B', ']'});

	sensors->receiveMessage(message);
	delete message;

	//Calibration data
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ASSERT_EQ(1, dmCommStub.queueSize());
	CommNs::DMMessageIf* receivedMessage = dmCommStub.popMessage();
	ASSERT_NE(nullptr, receivedMessage);
	CommNs::TxMessage* txMessage = dynamic_cast<CommNs::TxMessage*> (receivedMessage);
	ASSERT_NE(nullptr, txMessage);
	//Calibration data
	EXPECT_EQ("[8:0.00:RoomName]", txMessage->txString());
	delete txMessage;

	const int MESSAGES = 40;
	for (int i = 0; i < MESSAGES; ++i)
	{
		message = new CommNs::RxMessage(std::vector<uint8_t> {0, '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0,
																				'[', '7', ':', 'A', 'B', ']'});
		sensors->receiveMessage(message);
		delete message;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//Will have received 20 set temperature's
	EXPECT_EQ(MESSAGES, dmCommStub.queueSize());

	//Delete all but 1 messages
	for (int i = 0; i < MESSAGES - 1; ++i)
	{
		receivedMessage = dmCommStub.popMessage();
		delete receivedMessage;
	}

	ASSERT_EQ(1, dmCommStub.queueSize());
	receivedMessage = dmCommStub.popMessage();
	ASSERT_NE(nullptr, receivedMessage);
	txMessage = dynamic_cast<CommNs::TxMessage*> (receivedMessage);
	ASSERT_NE(nullptr, txMessage);
	EXPECT_EQ("[5:10.0]", txMessage->txString());
	delete txMessage;


	// One up
	message = new CommNs::RxMessage(std::vector<uint8_t> {0, '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0,
																					'[', '6', ':', 'A', 'B', ']'});
	sensors->receiveMessage(message);
	delete message;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	ASSERT_EQ(1, dmCommStub.queueSize());
	receivedMessage = dmCommStub.popMessage();
	ASSERT_NE(nullptr, receivedMessage);
	txMessage = dynamic_cast<CommNs::TxMessage*> (receivedMessage);
	ASSERT_NE(nullptr, txMessage);
	EXPECT_EQ("[5:10.2]", txMessage->txString());
	delete txMessage;


	delete router;
	delete commServerStub;
	delete filter;
	delete sensors;
}

