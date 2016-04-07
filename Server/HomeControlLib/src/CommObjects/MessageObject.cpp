/*
 * MessageObject.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: koen
 */

#include <CommObjects/MessageObject.h>
#include "json/json.h"
#include <sstream>

namespace CommNs {

MessageObject::MessageObject(const std::string& json):
	mMessage(),
	mTimeStamp()
{
	Json::Value jsonRoot;
    Json::Reader reader;
    if (reader.parse(json, jsonRoot, false))
    {
    	if (jsonRoot.isMember("Message"))
    	{
    		mMessage = jsonRoot["Message"].asString();
    	}
    	if (jsonRoot.isMember("TimeStamp"))
    	{
    		// (1459800687043+
    		std::string timeString = jsonRoot["TimeStamp"].asString();
    		auto pos1 = timeString.find('(');
    		auto pos2 = timeString.find('+');
    		if (pos1 > 0 && pos2 > 0)
    		{
    			++pos1;
    			//.NET Json time is milliseconds; ignoring timezone (we are in the same timezone)
    			std::string epocSeconds = timeString.substr(pos1, pos2-pos1);
    			mTimeStamp = std::stoll(epocSeconds) / 1000;
    		}
    	}
    }}

MessageObject::~MessageObject()
{
}
uint8_t MessageObject::objectId() const
{
	return 11;
}

std::string MessageObject::toString() const
{
	std::stringstream ss;
	ss << "Message: " << mMessage << ", Time: " <<  std::asctime(std::localtime(&mTimeStamp));
	return ss.str();
}
} /* namespace CommNs */
