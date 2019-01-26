/*
 * TemperatureDown.cpp
 *
 *  Created on: Jan 26, 2019
 *      Author: koen
 */

#include <CommObjects/TemperatureDown.h>
#include "json/json.h"

namespace CommNs {

TemperatureDown::TemperatureDown():
	mRoomId()
{
}
TemperatureDown::TemperatureDown(const std::string& json):
	mRoomId()
{
	Json::Value jsonRoot;
    Json::Reader reader;
    if (reader.parse(json, jsonRoot, false))
    {
    	if (jsonRoot.isMember("roomId"))
    	{
    		mRoomId = jsonRoot["roomId"].asString();
    	}
    }
}

TemperatureDown::~TemperatureDown()
{
}

uint8_t TemperatureDown::objectId() const
{
	return 31;
}

std::string TemperatureDown::toString() const
{
	return "";
}

std::string TemperatureDown::json() const
{
	return "";
}
} /* namespace CommNs */
