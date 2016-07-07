/*
 * SerialTest.cpp
 *
 *  Created on: May 29, 2016
 *      Author: koen
 */


#include "Comm/Serial.h"
#include "Comm/SerialListenerIf.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>
#include <atomic>

class SerialListener: public CommNs::SerialListenerIf
{
public:
	SerialListener(): mLastLine(), mLineCount() {} ;
	~SerialListener() {};

	void receiveLine(const std::string& line)
	{
		LOG(INFO) << "Received: " << line;
		mLastLine = line;
		++mLineCount;
	}
	void receiveData(const std::vector<uint8_t>& data) {};
	std::string mLastLine;
	std::atomic_int mLineCount;
};

TEST(Serial, Receive)
{
	CommNs::Serial* serial = new CommNs::Serial("/dev/ttyAMA0", 38400);
	SerialListener serialListener;
	serial->registerSerialListener(&serialListener);
	std::this_thread::sleep_for(std::chrono::seconds(60));

	serial->unRegisterSerialListener();
	delete serial;
}
