/*
 * RoomControl.cpp
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#include <Logic/RoomControl.h>
#include "RoomListenerIf.h"
#include "HeaterListenerIf.h"
#include "ThermostatIf.h"
#include <glog/logging.h>
#include <ctime>

namespace
{
const int WAIT_FOR_WORK_TIMEOUT_SEC = 5;
const double SET_TEMP_DELTA = 0.2;

const double MIN_SET_TEMPERATURE = 10.0;
const double MAX_SET_TEMPERATURE = 26.0;
}

namespace LogicNs
{

RoomControl::RoomControl(const std::string& roomId, uint8_t heaterOutput, const std::string& roomName, RoomListenerIf* roomListener, std::shared_ptr<ThermostatIf> thermostat, HeaterListenerIf* heaterListener):
	mRoomId(roomId),
	mHeaterOutput(heaterOutput),
	mRoomName(roomName),
	mRoomListener(roomListener),
	mThermostat(thermostat),
	mHeaterListener(heaterListener),
	mRoomTemperature(21.0),
	mSetTemperature(21),
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
	if (mThermostat)
	{
		mThermostat->registerListener(this);
		mThermostat->setPointChanged(std::time(0), mSetTemperature);
	}
	startWorkerThread();
}

RoomControl::~RoomControl()
{
	if (mThermostat)
	{
		mThermostat->unRegisterListener();
	}
	stopWorkerThread();
}

void RoomControl::heaterOn()
{
	LOG(INFO) << "Room: " << mRoomId << ", heater on";
	if (mRoomListener)
	{
		mRoomListener->heaterOn(mRoomId);
	}
	if (mHeaterListener)
	{
		mHeaterListener->heaterOn(mHeaterOutput);
	}
}

void RoomControl::heaterOff()
{
	LOG(INFO) << "Room: " << mRoomId << ", heater off";
	if (mRoomListener)
	{
		mRoomListener->heaterOff(mRoomId);
	}
	if (mHeaterListener)
	{
		mHeaterListener->heaterOff(mHeaterOutput);
	}
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
		bool setChanged = false;
		bool tempChanged = false;
		auto until = std::chrono::system_clock::now() +std::chrono::seconds(WAIT_FOR_WORK_TIMEOUT_SEC);
		std::unique_lock<std::mutex> lock(mConditionVarMutex);
		mWaitForWorkCondVar.wait_until(lock, until,[&]{return (bool)(mWorkReceived > 0);});

		if ((mWorkReceived > 0) && mWorkerThreadRunning)
		{
			while (mUpRequested)
			{
				VLOG(1) << "Processing Temperature Up";
				setChanged = true;

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
				setChanged = true;

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
				tempChanged = true;

				if (mRoomListener)
				{
					mRoomListener->temperatureChanged(mRoomId, mRoomTemperature);
				}
				mTempReceived = false;
			}

			--mWorkReceived;
		}

		if (mThermostat)
		{
			if (setChanged)
			{
				mThermostat->setPointChanged(std::time(0), mSetTemperature);
			}
			if (tempChanged)
			{
				LOG(INFO) << "Temp for room: " << mRoomId << ", temperature: " << mRoomTemperature;
				mThermostat->temperatureChanged(std::time(0), mRoomTemperature);
			}
		}
	}
}
} /* namespace LogicNs */
