/*
 * RoomControl.cpp
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#include <Logic/RoomControl.h>
#include "RoomListenerIf.h"

namespace LogicNs {

RoomControl::RoomControl(const std::string& roomId, const std::string& roomName, RoomListenerIf* roomListener):
	mRoomId(roomId),
	mRoomName(roomName),
	mRoomListener(roomListener),
	mRoomTemperature(21.0),
	mSetTemperature(16),
	mDataMutex()
{
}

RoomControl::~RoomControl()
{
}

void RoomControl::roomTemperature(double temperature)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	mRoomTemperature = temperature;
	if (mRoomListener)
	{
		mRoomListener->temperatureChanged(mRoomId, mRoomTemperature);
	}
}

void RoomControl::setPointUp()
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	mSetTemperature += 0.2;
	if (mRoomListener)
	{
		mRoomListener->setPointChanged(mRoomId, mSetTemperature);
	}
}

void RoomControl::setPointDown()
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	mSetTemperature -= 0.2;
	if (mRoomListener)
	{
		mRoomListener->setPointChanged(mRoomId, mSetTemperature);
	}
}
} /* namespace LogicNs */
