/*
 * RoomControlTest.cpp
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#include "Logic/RoomControl.h"
#include "Logic/RoomListenerIf.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>
#include <atomic>

class RoomListenerStub: public LogicNs::RoomListenerIf
{
public:
	virtual ~RoomListenerStub() {};

	void temperatureChanged(const std::string& roomId, double temperature)
	{
		mLastTemperature = temperature;
	}
	void setPointChanged(const std::string& roomId, double setTemperature)
	{
		mLastSetTemperature = setTemperature;
	}

	void heaterOn(const std::string& roomId)
	{
		mHeaterOnRoomId = roomId;
	}

	virtual void heaterOff(const std::string& roomId)
	{
		mHeaterOffRoomId = roomId;
	}
	double mLastTemperature;
	double mLastSetTemperature;
	std::string mHeaterOnRoomId;
	std::string mHeaterOffRoomId;
};

TEST(RoomControl, Constructor)
{
	LogicNs::RoomControl* room = new LogicNs::RoomControl("id", "name", nullptr);
	EXPECT_EQ("id", room->roomId());
	EXPECT_EQ("name", room->roomName());
	delete room;
}

TEST(RoomControl, TemperatureChange)
{
	RoomListenerStub roomListener;
	LogicNs::RoomControl* room = new LogicNs::RoomControl("id", "name", &roomListener);

	room->roomTemperature(23.5);
	EXPECT_EQ(23.5, roomListener.mLastTemperature);

	room->roomTemperature(18.5);
	EXPECT_EQ(18.5, roomListener.mLastTemperature);

	delete room;
}

TEST(RoomControl, SetUp)
{
	RoomListenerStub roomListener;
	LogicNs::RoomControl* room = new LogicNs::RoomControl("id", "name", &roomListener);

	room->setPointUp();
	EXPECT_TRUE(abs(roomListener.mLastSetTemperature - 16.2) < 0.01);

	room->setPointUp();
	EXPECT_TRUE(abs(roomListener.mLastSetTemperature - 16.4) < 0.01);

	delete room;
}

TEST(RoomControl, SetDown)
{
	RoomListenerStub roomListener;
	LogicNs::RoomControl* room = new LogicNs::RoomControl("id", "name", &roomListener);

	room->setPointDown();
	EXPECT_TRUE(abs(roomListener.mLastSetTemperature - 15.8) < 0.01);

	room->setPointDown();
	EXPECT_TRUE(abs(roomListener.mLastSetTemperature - 15.6) < 0.01);

	delete room;
}
