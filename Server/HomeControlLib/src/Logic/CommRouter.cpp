/*
 * HomeControl.cpp
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#include <Logic/CommRouter.h>
#include "Comm/CommServerIf.h"
#include "Comm/TemperatureSensorsIf.h"
#include "CommObjects/RoomTemperature.h"
#include "CommObjects/RoomList.h"
#include "Logic/RoomControl.h"
#include "DAL/HomeControlDalIf.h"
#include <glog/logging.h>
#include <algorithm>

namespace LogicNs {

CommRouter::CommRouter(DalNs::HomeControlDalIf* dal, CommNs::CommServerIf* server, CommNs::TemperatureSensorsIf* sensors):
	mDal(dal),
	mCommServer(server),
	mSensors(sensors),
	mConnnectedClients(),
	mDataMutex(),
	mRooms()
{
	if (mCommServer) mCommServer->registerCommListener(this);
	if (mSensors) mSensors->registerTemperatureListener(this);
}

CommRouter::~CommRouter()
{
	if (mSensors) mSensors->unRegisterTemperatureListener(this);
	if (mCommServer) mCommServer->unRegisterCommListener(this);

	for (const auto& room: mRooms)
	{
		delete room.second;
	}
}

void CommRouter::temperatureChanged(const std::string& roomId, double temperature)
{
	if (mCommServer)
	{
		// Temperature has changed in the room; send it to all connected CommClients
		std::lock_guard<std::recursive_mutex> lg(mDataMutex);
		for(const auto& client: mConnnectedClients)
		{
			CommNs::RoomTemperature* roomTemperature = new CommNs::RoomTemperature(roomId, temperature);

			//CommServer takes ownership of the object (and free's the object)
			mCommServer->sendObject(client, roomTemperature);
		}
	}
}

void CommRouter::setPointChanged(const std::string& roomId, double setTemperature)
{

}

void CommRouter::heaterOn(const std::string& roomId)
{

}

void CommRouter::heaterOff(const std::string& roomId)
{

}

void CommRouter::clientConnected(const std::string& name)
{
	std::lock_guard<std::recursive_mutex> lg(mDataMutex);
	mConnnectedClients.insert(name);
	LOG(INFO) << "Client connected: " << name;
	sendRooms();
}

void CommRouter::clientDisConnected(const std::string& name)
{
	std::lock_guard<std::recursive_mutex> lg(mDataMutex);
	mConnnectedClients.erase(name);
}

void CommRouter::receiveObject(const std::string name, const CommNs::CommObjectIf* object)
{

}

void CommRouter::sensorStarted(const std::string& sensorId)
{
	std::lock_guard<std::recursive_mutex> lg(mDataMutex);

	RoomControl* room = findRoomBySensorId(sensorId);
	if (room)
	{
		LOG(INFO) << "Sensor for room: '" << room->roomName() << "' started";
	}
}

void CommRouter::sensorTemperature(const std::string& sensorId, double temperature)
{
	std::lock_guard<std::recursive_mutex> lg(mDataMutex);

	RoomControl* room = findRoomBySensorId(sensorId);
	if (room)
	{
		room->roomTemperature(temperature);
	}
}

void CommRouter::sensorSetTemperatureUp(const std::string& sensorId)
{
	std::lock_guard<std::recursive_mutex> lg(mDataMutex);

	RoomControl* room = findRoomBySensorId(sensorId);
	if (room)
	{
		room->setPointUp();
	}
}

void CommRouter::sensorSetTemperatureDown(const std::string& sensorId)
{
	std::lock_guard<std::recursive_mutex> lg(mDataMutex);

	RoomControl* room = findRoomBySensorId(sensorId);
	if (room)
	{
		room->setPointDown();
	}
}

RoomControl* CommRouter::findRoomByRoomId(const std::string& roomId, bool useDatabase)
{
	for (const auto& room: mRooms)
	{
		if (room.second->roomId() == roomId)
		{
			return room.second;
		}
	}

	if (mDal && useDatabase)
	{
		LOG(INFO) << "RoomId: '" << roomId << "' not found, trying database";
		DalNs::RoomConfig* roomConfig = mDal->findRoomByRoomId(roomId);
		if (roomConfig)
		{
			RoomControl* roomControl = new RoomControl(roomConfig->RoomId, roomConfig->RoomName, this);
			mRooms.push_back(std::make_pair(std::set<std::string>(), roomControl));

			sendRooms();
			delete roomConfig;
			return roomControl;
		}
		else
		{
			LOG(ERROR) << "Room config not found in database";
		}
	}

	return nullptr;
}

RoomControl* CommRouter::findRoomBySensorId(const std::string& sensorId)
{
	for (const auto& room: mRooms)
	{
		if (std::find(room.first.cbegin(), room.first.cend(), sensorId) != room.first.cend())
		{
			return room.second;
		}
	}
	LOG(INFO) << "No room found for SensorId: '" << sensorId << "' not found, trying database";

	DalNs::RoomConfig* roomConfig = mDal->findRoomBySensorId(sensorId);
	if (roomConfig)
	{
		// Could be that the room was already created, but not with our sensorId (Room with multiple sensors)
		RoomControl* roomControl = findRoomByRoomId(roomConfig->RoomId, false);
		if (roomControl == nullptr)
		{   // Room does not exist; construct the room
			roomControl = new RoomControl(roomConfig->RoomId, roomConfig->RoomName, this);
			mRooms.push_back(std::make_pair(std::set<std::string>(), roomControl));
		}
		addSensorToRoom(roomConfig->RoomId, sensorId);

		sendRooms();
		delete roomConfig;
		return roomControl;
	}
	else
	{
		LOG(ERROR) << "Room config not found in database";
	}

	return nullptr;
}

void CommRouter::addSensorToRoom(const std::string& roomId, const std::string& sensorId)
{
	for (auto& room: mRooms)
	{
		if (room.second->roomId() == roomId)
		{
			room.first.insert(sensorId);
		}
	}
}

void CommRouter::sendRooms()
{
	if (mCommServer)
	{
		for(const auto& client: mConnnectedClients)
		{
			CommNs::RoomList* roomList = new CommNs::RoomList();
			for (auto& room: mRooms)
			{
				CommNs::Room newRoom;
				newRoom.RoomId = room.second->roomId();
				newRoom.RoomName = room.second->roomName();
				newRoom.RoomTemperature = room.second->roomTemperature();
				newRoom.SetTemperature = room.second->setTemperature();
				roomList->addRoom(newRoom);
			}

			//CommServer takes ownership of the object (and free's the object)
			mCommServer->sendObject(client, roomList);
		}
	}
}
} /* namespace LogicNs */
