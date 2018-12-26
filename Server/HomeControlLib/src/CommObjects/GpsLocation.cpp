/*
 * GpsLocation.cpp
 *
 *  Created on: Apr 4, 2016
 *      Author: koen
 */

#include "GpsLocation.h"
#include "json/json.h"
#include <sstream>
#include <iostream>

namespace CommNs {

GpsLocation::GpsLocation(const std::string& json):
	mLatitude(0),
	mLongitude(0),
	mAccuracy(0),
	mTimeStamp()
{
	Json::Value jsonRoot;
    Json::Reader reader;
    if (reader.parse(json, jsonRoot, false))
    {
    	if (jsonRoot.isMember("Accuracy"))
    	{
    		mAccuracy = jsonRoot["Accuracy"].asDouble();
    	}
    	if (jsonRoot.isMember("Latitude"))
    	{
    		mLatitude = jsonRoot["Latitude"].asDouble();
    	}
    	if (jsonRoot.isMember("Longitude"))
    	{
    		mLongitude = jsonRoot["Longitude"].asDouble();
    	}
    	if (jsonRoot.isMember("BatteryLevel"))
    	{
    		mBatteryLevel = jsonRoot["BatteryLevel"].asDouble();
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
    		//mTimeStamp = jsonRoot["TimeStamp"].asLargestInt();
    	}
    }
}

GpsLocation::~GpsLocation()
{
}

uint8_t GpsLocation::objectId() const
{
	return 10;
}

std::string GpsLocation::toString() const
{
	std::stringstream ss;
	ss << "Lat: " << mLatitude << ", Lon: " << mLongitude << ", Accuracy: " << mAccuracy << ", Battery: " << mBatteryLevel << ", Time: " <<  std::asctime(std::localtime(&mTimeStamp));
	return ss.str();
}

} /* namespace CommNs */
