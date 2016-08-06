/*
 * LiveTest.cpp
 *
 *  Created on: Apr 5, 2016
 *      Author: koen
 */


#include "Comm/Server.h"
#include "Comm/SocketFactory.h"
#include "Comm/CommListenerIf.h"
#include "CommObjects/CommObjectIf.h"
#include "Comm/Serial.h"
#include "Comm/DMFrameProcessor.h"
#include "Comm/DMFrameListenerIf.h"
#include "Comm/DMComm.h"
#include "Comm/DMMessages.h"
#include "Comm/TemperatureSensors.h"
#include "Logic/TemperatureListenerIf.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include "gflags/gflags.h"
#include <string>
#include <thread>
#include <iostream>

DEFINE_string(serial, "/dev/ttyUSB0", "Serial port to use");
namespace
{
class CommListenerStub: public CommNs::CommListenerIf
{
public:
	CommListenerStub() {};
	~CommListenerStub() {};
	void clientConnected(const std::string& name) {};
	void clientDisConnected(const std::string& name) {};

	void receiveObject(const std::string name, const CommNs::CommObjectIf* object)
	{
		std::cout << name << ": " << object->toString() << std::endl;
	};

};

class DMFrameListenerStub: public CommNs::DMFrameListenerIf
{
public:
	virtual ~DMFrameListenerStub() {};
	void receiveFrame(const std::vector<uint8_t>& data)
	{
		mLastFrame = data;
	};
	std::vector<uint8_t> mLastFrame;
};

class TemperatureListenerStub: public LogicNs::TemperatureListenerIf
{
public:
	TemperatureListenerStub(): mLastId(), mLastTemperature() {};
	~TemperatureListenerStub() {};

	void sensorStarted(const std::string& sensorId)
	{
		mLastId = sensorId;
	}
	void sensorTemperature(const std::string& sensorId, double temperature)
	{
		LOG(INFO) << "Temperature, sensor: " << sensorId << ", Temp: " << temperature;
		mLastId = sensorId;
		mLastTemperature = temperature;
	}
	void sensorSetTemperatureUp(const std::string& sensorId)
	{
		mLastId = sensorId;
	}
	void sensorSetTemperatureDown(const std::string& sensorId)
	{
		mLastId = sensorId;
	}
	std::string mLastId;
	double mLastTemperature;
};
}

TEST(LiveTest, ServerListening)
{
	CommListenerStub* commListener = new CommListenerStub;
	CommNs::SocketFactory* factory = new CommNs::SocketFactory;
	CommNs::Server* server = new CommNs::Server(factory, 5678);
	server->registerCommListener(commListener);
	std::this_thread::sleep_for(std::chrono::seconds(600));

	delete server;
	delete factory;
	delete commListener;
}

TEST(LiveTest, Serial)
{
	CommNs::Serial serial(FLAGS_serial, 38400);
	serial.openSerial();
	std::this_thread::sleep_for(std::chrono::seconds(1));

	//std::vector<uint8_t> testData {0x7E, 0x00, 0x04, 0x08, 0x01, 0x61, 0x74, 0x21};
	std::vector<uint8_t> testData {'+', '+', '+'};
	serial.writeData(testData);
	std::this_thread::sleep_for(std::chrono::seconds(3));

	std::vector<uint8_t> testVL {'A', 'T', 'V', 'L', 0x0D};
	serial.writeData(testVL);
	std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST(LiveTest, SendApiCommand)
{
	CommNs::Serial serial(FLAGS_serial, 38400);
	serial.openSerial();
	//std::this_thread::sleep_for(std::chrono::milliseconds());
	CommNs::DMFrameProcessor processor(&serial);
	CommNs::DMComm dmComm(&processor, 0x0B, {0x12, 0x13});
	CommNs::DMMessageIf* received = dmComm.sendATCmd("SH", {}, 100);
	EXPECT_NE(nullptr, received);
	if (received != nullptr)
	{
		LOG(INFO) << received->toString();

		delete received;
	}

	received = dmComm.sendATCmd("SL", {}, 100);
	EXPECT_NE(nullptr, received);
	if (received != nullptr)
	{
		LOG(INFO) << received->toString();

		delete received;
	}

}

TEST(LiveTest, Server)
{
	CommNs::Serial serial(FLAGS_serial, 38400);
	serial.openSerial();
	//std::this_thread::sleep_for(std::chrono::milliseconds());
	CommNs::DMFrameProcessor processor(&serial);
	CommNs::DMComm dmComm(&processor, 0x0B, {0x12, 0x13});

	std::this_thread::sleep_for(std::chrono::seconds(3000));
}

TEST(LiveTest, TempSensorsServer)
{
	CommNs::Serial serial(FLAGS_serial, 38400);
	serial.openSerial();
	//std::this_thread::sleep_for(std::chrono::milliseconds());
	CommNs::DMFrameProcessor processor(&serial);
	CommNs::DMComm* dmComm = new CommNs::DMComm(&processor, 0x0B, {0x12, 0x13});
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(dmComm);
	TemperatureListenerStub tempListenerStub;
	sensors->registerTemperatureListener(&tempListenerStub);

	std::this_thread::sleep_for(std::chrono::seconds(3000));

	sensors->unRegisterTemperatureListener(&tempListenerStub);
	delete sensors;
	delete dmComm;
}

TEST(LiveTest, Client)
{
	CommNs::Serial*  serial = new CommNs::Serial(FLAGS_serial, 38400);
	serial->openSerial();
	//std::this_thread::sleep_for(std::chrono::milliseconds());
	CommNs::DMFrameProcessor* processor = new  CommNs::DMFrameProcessor(serial);
	CommNs::DMComm* dmComm= new CommNs::DMComm(processor, 0x0B, {0x12, 0x13});
	std::this_thread::sleep_for(std::chrono::seconds(10));
	dmComm->sendATCmd("ND", {});
	std::this_thread::sleep_for(std::chrono::seconds(20));

	CommNs::TxMessage* sendData = new CommNs::TxMessage({'T', 'e', 's', 't'}, {0x00, 0x13, 0xA2, 0x00, 0x40, 0xD9, 0xD5, 0xDB});
//	CommNs::TxMessage* sendData = new CommNs::TxMessage({'T', 'e', 's', 't'}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF});
	dmComm->sendMessage(sendData);

	std::this_thread::sleep_for(std::chrono::seconds(5));
	delete dmComm;
	delete processor;
	delete serial;
}
