/*
 * TemperatureUp.cpp
 *
 *  Created on: Jan 26, 2019
 *      Author: koen
 */

#include <CommObjects/TemperatureUp.h>
#include "json/json.h"

namespace CommNs {

TemperatureUp::TemperatureUp():
	mRoomId()
{
}

TemperatureUp::TemperatureUp(const std::string& json):
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

TemperatureUp::~TemperatureUp()
{
}

uint8_t TemperatureUp::objectId() const
{
	return 30;
}

std::string TemperatureUp::toString() const
{
	return "";
}

std::string TemperatureUp::json() const
{
	return "";
}

} /* namespace CommNs */
