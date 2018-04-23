/*
 * TimerTest.cpp
 *
 *  Created on: Apr 2, 2018
 *      Author: koen
 */

#include "Comm/TimeListenerIf.h"
#include "Logic/Timer.h"
#include "Logic/SystemClockIf.h"
#include "Logic/SystemClock.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <thread>

class TimeListenerStub: public CommNs::TimeListenerIf
{
public:
	TimeListenerStub(): mLastTime() {};
	virtual ~TimeListenerStub() {};
	void writeTime(const std::string& time)
	{
		mLastTime = time;
	}
	std::string mLastTime;
};

class SystemClockStub: public LogicNs::SystemClockIf
{
public:
	SystemClockStub(): mTime() {};
	virtual ~SystemClockStub() {};
	LogicNs::LocalTime localTime() {return mTime;};
	LogicNs::LocalTime mTime;
};

TEST(TimerTest, Constructor)
{
	SystemClockStub systemClock;
	LogicNs::Timer* timer = new LogicNs::Timer(systemClock, nullptr);
	delete timer;
}

TEST(TimerTest, WriteTimeAtStartup)
{
	SystemClockStub systemClock;
	systemClock.mTime.mHour = 1;
	systemClock.mTime.mMin = 10;
	EXPECT_EQ(1, systemClock.localTime().mHour);
	EXPECT_EQ(10, systemClock.localTime().mMin);

	TimeListenerStub listener;
	LogicNs::Timer* timer = new LogicNs::Timer(systemClock, &listener);
	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ("1:10", listener.mLastTime);

	systemClock.mTime.mHour = 2;
	systemClock.mTime.mMin = 1;

	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ("2:01", listener.mLastTime);

	systemClock.mTime.mHour = 17;
	systemClock.mTime.mMin = 5;

	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ("17:05", listener.mLastTime);

	delete timer;
}
