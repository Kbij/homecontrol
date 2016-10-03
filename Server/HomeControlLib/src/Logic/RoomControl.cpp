/*
 * RoomControl.cpp
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#include <Logic/RoomControl.h>
#include "RoomListenerIf.h"
#include <iostream>

namespace
{
const int WAIT_FOR_WORK_TIMEOUT_SEC = 5;
const double SET_TEMP_DELTA = 0.2;
}

namespace LogicNs
{

RoomControl::RoomControl(const std::string& roomId, const std::string& roomName, RoomListenerIf* roomListener):
	mRoomId(roomId),
	mRoomName(roomName),
	mRoomListener(roomListener),
	mRoomTemperature(21.0),
	mSetTemperature(16),
	mWorkerThread(nullptr),
	mWorkerThreadRunning(false),
	mUpRequested(0),
	mDownRequested(0),
	mTempReceived(false),
    mWaitForWorkCondVar(),
    mWorkReceived(0),
	mConditionVarMutex(),
	mDataMutex()
{
	startWorkerThread();
}

RoomControl::~RoomControl()
{
	stopWorkerThread();
}

void RoomControl::roomTemperature(double temperature)
{
	std::lock_guard<std::recursive_mutex> lg(mDataMutex);
	mRoomTemperature = temperature;
	mTempReceived = true;
	++mWorkReceived;
	mWaitForWorkCondVar.notify_one();
}


void RoomControl::setPointUp()
{
	++mUpRequested;
	++mWorkReceived;
	mWaitForWorkCondVar.notify_one();
}

void RoomControl::setPointDown()
{
	++mDownRequested;
	++mWorkReceived;
	mWaitForWorkCondVar.notify_one();
}

void RoomControl::startWorkerThread()
{
	if (!mWorkerThreadRunning)
	{
		mWorkerThreadRunning = true;
		mWorkerThread = new std::thread(&RoomControl::workerThread, this);
	}
}

void RoomControl::stopWorkerThread()
{
	mWorkerThreadRunning = false;
	++mWorkReceived;
	mWaitForWorkCondVar.notify_one();

    if (mWorkerThread)
    {
    	mWorkerThread->join();
    	delete mWorkerThread;
    	mWorkerThread = nullptr;
    }
}

void RoomControl::workerThread()
{
	while(mWorkerThreadRunning)
	{
		auto until = std::chrono::system_clock::now() +std::chrono::seconds(WAIT_FOR_WORK_TIMEOUT_SEC);
		std::unique_lock<std::mutex> lock(mConditionVarMutex);
		mWaitForWorkCondVar.wait_until(lock, until,[&]{return (bool)(mWorkReceived > 0);});
		if ((mWorkReceived > 0) && mWorkerThreadRunning)
		{
			while (mUpRequested)
			{
				--mUpRequested;
				mSetTemperature += SET_TEMP_DELTA;
				if (mRoomListener)
				{
					mRoomListener->setPointChanged(mRoomId, mSetTemperature);

					// Thermostat function; need a seperate object
					if (mSetTemperature >  mRoomTemperature)
					{
						mRoomListener->heaterOn(mRoomId);
					}
					else
					{
						mRoomListener->heaterOff(mRoomId);
					}
				}
			}

			while (mDownRequested)
			{
				--mDownRequested;
				mSetTemperature -= SET_TEMP_DELTA;
				if (mRoomListener)
				{
					mRoomListener->setPointChanged(mRoomId, mSetTemperature);

					// Thermostat function; need a seperate object
					if (mSetTemperature >  mRoomTemperature)
					{
						mRoomListener->heaterOn(mRoomId);
					}
					else
					{
						mRoomListener->heaterOff(mRoomId);
					}
				}
			}
			if (mTempReceived)
			{
				if (mRoomListener)
				{
					mRoomListener->temperatureChanged(mRoomId, mRoomTemperature);

					// Thermostat function; need a seperate object
					if (mSetTemperature >  mRoomTemperature)
					{
						mRoomListener->heaterOn(mRoomId);
					}
					else
					{
						mRoomListener->heaterOff(mRoomId);
					}
				}
			}
		}
		--mWorkReceived;
	}
}
} /* namespace LogicNs */
