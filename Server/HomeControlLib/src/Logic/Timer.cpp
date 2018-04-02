/*
 * Timer.cpp
 *
 *  Created on: Apr 2, 2018
 *      Author: koen
 */

#include <Logic/Timer.h>
#include "SystemClockIf.h"
#include "Comm/TimeListenerIf.h"
#include <sstream>
#include <iomanip>

namespace
{
const int MAINTENANCE_INTERVAL_MS = 1000;
}

namespace LogicNs {

Timer::Timer(SystemClockIf& systemClock, CommNs::TimeListenerIf* timeListener):
	mSystemClock(systemClock),
	mTimeListener(timeListener),
	mThread(),
	mTimerThreadRunning()
{
	startTimerThread();
}

Timer::~Timer()
{
	stopTimerThread();
}

void Timer::startTimerThread()
{
	if (!mTimerThreadRunning)
	{
		mTimerThreadRunning = true;
		mThread = new std::thread(&Timer::timerThread, this);
	}
}

void Timer::stopTimerThread()
{
	mTimerThreadRunning = false;

    if (mThread)
    {
    	mThread->join();
    	delete mThread;
    	mThread = nullptr;
    }
}

void Timer::timerThread()
{
	while(mTimerThreadRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(MAINTENANCE_INTERVAL_MS));
		if (mTimeListener)
		{
			std::stringstream ss;
			LocalTime time = mSystemClock.localTime();
			ss << (int) time.mHour << ":" << std::setfill('0') << std::setw(2) << (int) time.mMin;
			mTimeListener->writeTime(ss.str());
		}
	}

}

} /* namespace LogicNs */
