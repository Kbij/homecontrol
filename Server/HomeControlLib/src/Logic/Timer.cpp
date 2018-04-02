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
const int BROADCAST_REPEAT = 3;
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
	int lastMinutes = -1;
	while(mTimerThreadRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(MAINTENANCE_INTERVAL_MS));
		LocalTime time = mSystemClock.localTime();

		if (mTimeListener && time.mMin != lastMinutes)
		{
			for(int i = 0; i < BROADCAST_REPEAT; ++i)
			{
				std::stringstream ss;
				ss << (int) time.mHour << ":" << std::setfill('0') << std::setw(2) << (int) time.mMin;
				mTimeListener->writeTime(ss.str());
				std::this_thread::sleep_for(std::chrono::milliseconds(MAINTENANCE_INTERVAL_MS));
			}

			lastMinutes = time.mMin;
		}
	}

}

} /* namespace LogicNs */
