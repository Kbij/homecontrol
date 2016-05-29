/*
 * SerialTest.cpp
 *
 *  Created on: May 29, 2016
 *      Author: koen
 */


#include "Comm/Serial.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>

TEST(Serial, Receive)
{
	CommNs::Serial* serial = new CommNs::Serial("/dev/ttyAMA0", 9600);
	int i = 0;
	while (i < 10)
	{
		LOG(INFO) << serial->readLine();
		++i;
	}
	delete serial;
}
