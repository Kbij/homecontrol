/*
 * FrameProcessorTest.cpp
 *
 *  Created on: Apr 16, 2018
 *      Author: koen
 */


/*
0x7E, 0x00, 0x09, 0x88, 0x02, 0x53, 0x48, 0x00, 0x00, 0x7D, 0x33, 0xA2, 0x00

0x25 missing ?
*/
#include "Comm/DMFrameProcessor.h"
#include "Comm/DMFrameListenerIf.h"
#include <gtest/gtest.h>


namespace
{
class DMFrameListenerStub: public CommNs::DMFrameListenerIf
{
public:
	DMFrameListenerStub(): mReceivedFrame() {};
	virtual ~DMFrameListenerStub() {};

	void receiveFrame(const std::vector<uint8_t>& data) {mReceivedFrame = data;};
	std::vector<uint8_t> mReceivedFrame;
};
}

TEST(DMFrameProcessor, Constructor)
{
	CommNs::DMFrameProcessor* processor = new CommNs::DMFrameProcessor(nullptr);
	delete processor;
}

TEST(DMFrameProcessor, ReceiveFrame)
{
	DMFrameListenerStub* receiver = new DMFrameListenerStub;
	CommNs::DMFrameProcessor* processor = new CommNs::DMFrameProcessor(nullptr);
	processor->registerFrameListener(receiver);

	//Contains 0x7D; escaped character
	processor->receiveData({0x7E, 0x00, 0x09, 0x88, 0x02, 0x53, 0x48, 0x00, 0x00, 0x7D, 0x33, 0xA2, 0x00, 0x25, 0x11});
	EXPECT_EQ((size_t)0, receiver->mReceivedFrame.size());

	//Contains 0x7D; escaped character
	processor->receiveData({0x7E, 0x00, 0x09, 0x88, 0x02, 0x53, 0x48, 0x00, 0x00, 0x7D, 0x33, 0xA2, 0x00, 0x25});
	EXPECT_EQ((size_t)0, receiver->mReceivedFrame.size());

	delete processor;
}
