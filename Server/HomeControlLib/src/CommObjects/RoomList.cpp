/*
 * RoomList.cpp
 *
 *  Created on: Jun 10, 2016
 *      Author: koen
 */

#include <CommObjects/RoomList.h>
#include <json/json.h>

namespace CommNs {

RoomList::RoomList()
{

}

RoomList::RoomList(const std::string& json)
{

}

RoomList::~RoomList()
{

}

uint8_t RoomList::objectId() const
{
	return 21;
}

std::string RoomList::toString() const
{
	return "";
}

std::string RoomList::json() const
{
	Json::Value root;
	Json::Value roomArray;
	for(const auto& room: mRooms)
	{
		Json::Value jsonRoom;
		jsonRoom["roomId"] = room.RoomId;
		jsonRoom["name"] = room.RoomName;
		jsonRoom["roomTemperature"] = room.RoomTemperature;
		jsonRoom["setTemperature"] = room.SetTemperature;
		roomArray.append(jsonRoom);
	}

	root["rooms"] = roomArray;

	Json::FastWriter writer;
	return writer.write(root);
	return "";
}

void RoomList::addRoom(Room room)
{
	mRooms.push_back(room);
}
} /* namespace CommNs */
