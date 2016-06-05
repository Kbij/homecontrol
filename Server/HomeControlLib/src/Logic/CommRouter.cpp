/*
 * HomeControl.cpp
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#include <Logic/HomeControl.h>
#include "Comm/Server.h"
#include "Comm/Serial.h"

namespace LogicNs {

HomeControl::HomeControl(DalNs::HomeControlDalIf* dal, CommNs::Server* server):
	mDal(dal),
	mCommServer(server),
	mConnnectedClients(),
	mDataMutex()
{
	if (mCommServer) mCommServer->registerCommListener(this);
}

HomeControl::~HomeControl()
{
	if (mCommServer) mCommServer->unRegisterCommListener(this);
}

void HomeControl::temperatureChanged(const std::string& roomId, double temperature)
{

}

void HomeControl::setPointChanged(const std::string& roomId, double setTemperature)
{

}

void HomeControl::heaterOn(const std::string& roomId)
{

}

void HomeControl::heaterOff(const std::string& roomId)
{

}

void HomeControl::clientConnected(const std::string& name)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	mConnnectedClients.insert(name);
}

void HomeControl::clientDisConnected(const std::string& name)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	mConnnectedClients.erase(name);
}

void HomeControl::receiveObject(const std::string name, const CommNs::CommObjectIf* object)
{

}

void HomeControl::sensorStarted(const std::string& sensorId)
{

}

void HomeControl::sensorTemperature(const std::string& sensorId, double temperature)
{

}

void HomeControl::sensorSetTemperatureUp(const std::string& sensorId)
{

}

void HomeControl::sensorSetTemperatureDown(const std::string& sensorId)
{

}

RoomControl* HomeControl::findRoomByRoomId(const std::string& roomId)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	//for (const auto& room: mRooms)
	{
	//	if (room-)
	}
//	std::list<std::vector<std::string>, RoomControl*> mRooms;
}

RoomControl* findRoomBySensorId(const std::string& sensorId)
{

}

} /* namespace LogicNs */
