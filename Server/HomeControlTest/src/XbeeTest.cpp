/*
 * XbeeTest.cpp
 *
 *  Created on: Jul 2, 2016
 *      Author: koen
 */
#include <stdint.h>
#include "Comm/DMFrameListenerIf.h"
#include "Comm/DMFrameProcessor.h"
#include "Comm/SerialIf.h"
#include "Comm/SerialListenerIf.h"
#include "Comm/DMComm.h"
#include "Comm/DMCommListenerIf.h"
#include "Comm/DMMessages.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"

namespace
{
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

class SerialStub: public CommNs::SerialIf
{
public:
	SerialStub():
		mLastData()
	{
	};
	virtual ~SerialStub() {};

	void registerSerialListener(CommNs::SerialListenerIf* listener) {};
	void unRegisterSerialListener() {};

	void writeData(const std::vector<uint8_t>& data)
	{
		mLastData = data;
	};
	std::vector<uint8_t> mLastData;
};

class DMListenerStub: public CommNs::DMCommListenerIf
{
public:
	DMListenerStub():
		mMessage(),
		mMessageType(CommNs::DMMessageType::Unknown)
	{
	};
	virtual ~DMListenerStub() {};
	void receiveMessage(const CommNs::DMMessageIf* message)
	{
		//Warning !!!! You cannot use the pointer after this call; object is deleted by DMComm
		mMessage = message;
		mMessageType = mMessage->messageType();
	}
	const CommNs::DMMessageIf* mMessage;
	CommNs::DMMessageType mMessageType;
};
}

TEST(DMTests, FrameProcessor_ReceiveFullFrame)
{
	CommNs::DMFrameProcessor processor(nullptr);
	DMFrameListenerStub listenerStub;

	processor.registerFrameListener(&listenerStub);
	processor.receiveData({0x7E, 0x00, 0x04, 0x08, 0x01, 0x61, 0x74, 0x21});
	EXPECT_EQ(std::vector<uint8_t>({0x08, 0x01, 0x61, 0x74}), listenerStub.mLastFrame);
}

TEST(DMTests, FrameProcessor_ReceiveFullFrameAfterBogusData)
{
	CommNs::DMFrameProcessor processor(nullptr);
	DMFrameListenerStub listenerStub;

	processor.registerFrameListener(&listenerStub);
	processor.receiveData({0xFF, 0x01, 0x25, 0x7E, 0x00, 0x04, 0x08, 0x01, 0x61, 0x74, 0x21});
	EXPECT_EQ(std::vector<uint8_t>({0x08, 0x01, 0x61, 0x74}), listenerStub.mLastFrame);
}

TEST(DMTests, FrameProcessor_InvalidCrc)
{
	CommNs::DMFrameProcessor processor(nullptr);
	DMFrameListenerStub listenerStub;

	processor.registerFrameListener(&listenerStub);
	processor.receiveData({0x7E, 0x00, 0x04, 0x08, 0x01, 0x61, 0x74, 0x20});
	EXPECT_EQ((size_t) 0, listenerStub.mLastFrame.size());
}

TEST(DMTests, FrameProcessor_ReceiveFullFrame_2Packets)
{
	CommNs::DMFrameProcessor processor(nullptr);
	DMFrameListenerStub listenerStub;

	processor.registerFrameListener(&listenerStub);
	processor.receiveData({0x7E, 0x00});
	processor.receiveData({0x04, 0x08, 0x01, 0x61, 0x74, 0x21});
	EXPECT_EQ(std::vector<uint8_t>({0x08, 0x01, 0x61, 0x74}), listenerStub.mLastFrame);
}

TEST(DMTests, FrameProcessor_ReceiveZeroSize)
{
	CommNs::DMFrameProcessor processor(nullptr);
	DMFrameListenerStub listenerStub;

	processor.registerFrameListener(&listenerStub);
	processor.receiveData({0x7E, 0x00, 0x00, 0xFF});
	EXPECT_EQ((size_t) 0, listenerStub.mLastFrame.size());
}

TEST(DMTests, FrameProcessor_SendFrame)
{
	SerialStub serial;
	CommNs::DMFrameProcessor processor(&serial);

	processor.sendData({0x08, 0x01, 0x61, 0x74});

	EXPECT_EQ(std::vector<uint8_t>({0x7E, 0x00, 0x04, 0x08, 0x01, 0x61, 0x74, 0x21}), serial.mLastData);
}

TEST(DMTests, DMComm_ReceiveFrame)
{
	CommNs::DMComm dmComm(nullptr, 0x00, {0x00});
	dmComm.receiveFrame({0x08, 0x01, 0x61, 0x74});
}

TEST(DMTests, DMComm_ReceiveFrame_With_Listener)
{
	DMListenerStub listener;
	CommNs::DMComm dmComm(nullptr, 0x00, {0x00});
	dmComm.registerListener(&listener);
	dmComm.receiveFrame({0x88, 0x03, 0x43, 0x48, 0x00});
	ASSERT_NE(nullptr, listener.mMessage);
	EXPECT_EQ(CommNs::DMMessageType::ATResponse, listener.mMessageType);
}

TEST(DMTests, DMComm_ReceiveFrame_ModemStatus)
{
	DMListenerStub listener;
	CommNs::DMComm dmComm(nullptr, 0x00, {0x00});
	dmComm.registerListener(&listener);
	dmComm.receiveFrame({0x8A, 0x00});
	ASSERT_NE(nullptr, listener.mMessage);
	EXPECT_EQ(CommNs::DMMessageType::ModemStatus, listener.mMessageType);
}

TEST(DMTests, DMComm_ReceiveRxMessage)
{
	CommNs::DMComm dmComm(nullptr, 0x00, {0x00});
	dmComm.receiveFrame({0x90, 0x00, 0x13, 0xA2, 0x00, 0x40, 0xD9, 0xD5, 0xDB, 0xFF, 0xFE, 0xC1, 0x48, 0x65, 0x6C, 0x6C, 0x6F});
}




//0x88, 0x02, 0x53, 0x4C, 0x00, 0x40, 0xF3, 0x67, 0xEC
