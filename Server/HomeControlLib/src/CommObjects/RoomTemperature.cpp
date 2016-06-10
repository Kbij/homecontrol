/*
 * RoomTemperature.cpp
 *
 *  Created on: Jun 8, 2016
 *      Author: koen
 */

#include <CommObjects/RoomTemperature.h>
#include <sstream>
#include <json/json.h>

namespace CommNs {

RoomTemperature::RoomTemperature(const std::string& roomId, double temperature):
	mRoomId(roomId),
	mRoomTemperature(temperature)
{

}
RoomTemperature::RoomTemperature(const std::string& json):
	mRoomId(""),
	mRoomTemperature(0)
{
}

RoomTemperature::~RoomTemperature()
{
}

uint8_t RoomTemperature::objectId() const
{
	return 20;
}

std::string RoomTemperature::toString() const
{
	std::stringstream ss;
	ss << "Roomtemperature, Temperature: " <<  mRoomTemperature;
	return ss.str();
}

std::string RoomTemperature::json() const
{
	Json::Value root;
	root["roomId"] = mRoomId;
	root["roomTemperature"] = mRoomTemperature;

	Json::FastWriter writer;
	return writer.write(root);
}
} /* namespace CommNs */
