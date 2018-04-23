/*
 * RoomControl.cpp
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#include <Logic/RoomControl.h>
#include "RoomListenerIf.h"
#include <glog/logging.h>
#include <iostream>

namespace
{
const int WAIT_FOR_WORK_TIMEOUT_SEC = 5;
const double SET_TEMP_DELTA = 0.2;

const double MIN_SET_TEMPERATURE = 10.0;
const double MAX_SET_TEMPERATURE = 26.0;
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
	VLOG(1) << "(" << mRoomId << ") setpoint Up";
	++mUpRequested;
	++mWorkReceived;
	mWaitForWorkCondVar.notify_one();
}

void RoomControl::setPointDown()
{
	VLOG(1) << "(" << mRoomId << ") setpoint Down";
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

//	std::lock_guard<std::mutex> lg(mConditionVarMutex);
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
	LOG(INFO) << "Workerthread started for room: " << mRoomId;
	while(mWorkerThreadRunning)
	{
		bool workDone = false;
		auto until = std::chrono::system_clock::now() +std::chrono::seconds(WAIT_FOR_WORK_TIMEOUT_SEC);
		std::unique_lock<std::mutex> lock(mConditionVarMutex);
		mWaitForWorkCondVar.wait_until(lock, until,[&]{return (bool)(mWorkReceived > 0);});

		if ((mWorkReceived > 0) && mWorkerThreadRunning)
		{
			while (mUpRequested)
			{
				VLOG(1) << "Processing Temperature Up";
				workDone = true;

				--mUpRequested;
				mSetTemperature += SET_TEMP_DELTA;
				if (mSetTemperature > MAX_SET_TEMPERATURE) mSetTemperature = MAX_SET_TEMPERATURE;

				if (mRoomListener)
				{
					mRoomListener->setPointChanged(mRoomId, mSetTemperature);
				}
			}

			while (mDownRequested)
			{
				VLOG(1) << "Processing Temperature Down";
				workDone = true;

				--mDownRequested;
				mSetTemperature -= SET_TEMP_DELTA;
				if (mSetTemperature < MIN_SET_TEMPERATURE) mSetTemperature = MIN_SET_TEMPERATURE;

				if (mRoomListener)
				{
					mRoomListener->setPointChanged(mRoomId, mSetTemperature);
				}
			}
			if (mTempReceived)
			{
				workDone = true;

				if (mRoomListener)
				{
					mRoomListener->temperatureChanged(mRoomId, mRoomTemperature);
				}
				mTempReceived = false;
			}

			--mWorkReceived;
		}
		if (workDone)
		{
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
} /* namespace LogicNs */
