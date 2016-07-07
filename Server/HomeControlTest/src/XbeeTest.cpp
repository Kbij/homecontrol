/*
 * XbeeTest.cpp
 *
 *  Created on: Jul 2, 2016
 *      Author: koen
 */
#include <stdint.h>
#include "Comm/DMFrameListenerIf.h"
#include "Comm/DMFrameProcessor.h"
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
}

TEST(DMTests, FrameProcessor_ReceiveFullFrame)
{
	CommNs::DMFrameProcessor processor;
	DMFrameListenerStub listenerStub;

	processor.registerFrameListener(&listenerStub);
	processor.receiveData({0x7E, 0x00, 0x04, 0x08, 0x01, 0x61, 0x74, 0x21});
	EXPECT_EQ(std::vector<uint8_t>({0x08, 0x01, 0x61, 0x74}), listenerStub.mLastFrame);
}

TEST(DMTests, FrameProcessor_ReceiveFullFrameAfterBogusData)
{
	CommNs::DMFrameProcessor processor;
	DMFrameListenerStub listenerStub;

	processor.registerFrameListener(&listenerStub);
	processor.receiveData({0xFF, 0x01, 0x25, 0x7E, 0x00, 0x04, 0x08, 0x01, 0x61, 0x74, 0x21});
	EXPECT_EQ(std::vector<uint8_t>({0x08, 0x01, 0x61, 0x74}), listenerStub.mLastFrame);
}

TEST(DMTests, FrameProcessor_InvalidCrc)
{
	CommNs::DMFrameProcessor processor;
	DMFrameListenerStub listenerStub;

	processor.registerFrameListener(&listenerStub);
	processor.receiveData({0x7E, 0x00, 0x04, 0x08, 0x01, 0x61, 0x74, 0x20});
	EXPECT_EQ((size_t) 0, listenerStub.mLastFrame.size());
}

TEST(DMTests, FrameProcessor_ReceiveFullFrame_2Packets)
{
	CommNs::DMFrameProcessor processor;
	DMFrameListenerStub listenerStub;

	processor.registerFrameListener(&listenerStub);
	processor.receiveData({0x7E, 0x00});
	processor.receiveData({0x04, 0x08, 0x01, 0x61, 0x74, 0x21});
	EXPECT_EQ(std::vector<uint8_t>({0x08, 0x01, 0x61, 0x74}), listenerStub.mLastFrame);
}

TEST(DMTests, FrameProcessor_ReceiveZeroSize)
{
	CommNs::DMFrameProcessor processor;
	DMFrameListenerStub listenerStub;

	processor.registerFrameListener(&listenerStub);
	processor.receiveData({0x7E, 0x00, 0x00, 0xFF});
	EXPECT_EQ((size_t) 0, listenerStub.mLastFrame.size());
}
