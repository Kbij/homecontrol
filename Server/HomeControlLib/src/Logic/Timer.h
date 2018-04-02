/*
 * Timer.h
 *
 *  Created on: Apr 2, 2018
 *      Author: koen
 */

#ifndef LOGIC_TIMER_H_
#define LOGIC_TIMER_H_
//#include "SystemClockIf.h"
#include <thread>

namespace CommNs
{
class TimeListenerIf;
}
namespace LogicNs
{
class SystemClockIf;

class Timer
{
public:
	Timer(SystemClockIf& systemClock, CommNs::TimeListenerIf* timeListener);
	virtual ~Timer();
private:
	void startTimerThread();
	void stopTimerThread();
	void timerThread();

	SystemClockIf& mSystemClock;
	CommNs::TimeListenerIf* mTimeListener;
	std::thread* mThread;
	bool mTimerThreadRunning;
};

} /* namespace LogicNs */

#endif /* LOGIC_TIMER_H_ */
