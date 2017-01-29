/*
 * RoomTemperature.cpp
 *
 *  Created on: Jun 8, 2016
 *      Author: koen
 */

#include <CommObjects/SetTemperature.h>
#include <sstream>
#include <json/json.h>

namespace CommNs {

SetTemperature::SetTemperature(const std::string& roomId, double temperature):
	mRoomId(roomId),
	mSetTemperature(temperature)
{

}

SetTemperature::SetTemperature(const std::string& json):
	mRoomId(""),
	mSetTemperature(0)
{
}

SetTemperature::~SetTemperature()
{
}

uint8_t SetTemperature::objectId() const
{
	return 23;
}

std::string SetTemperature::toString() const
{
	std::stringstream ss;
	ss << "Settemperature, Temperature: " <<  mSetTemperature;
	return ss.str();
}

std::string SetTemperature::json() const
{
	Json::Value root;
	root["roomId"] = mRoomId;
	root["setTemperature"] = mSetTemperature;

	Json::FastWriter writer;
	return writer.write(root);
}
} /* namespace CommNs */
