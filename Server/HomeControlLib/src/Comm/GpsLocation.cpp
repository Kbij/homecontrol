/*
 * GpsLocation.cpp
 *
 *  Created on: Apr 4, 2016
 *      Author: koen
 */

#include "GpsLocation.h"
#include "json/json.h"
#include <sstream>

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
    	if (jsonRoot.isMember("TimeStamp"))
    	{
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
	ss << "Lat: " << mLatitude << ", Lon: " << mLongitude << ", Accuracy: " << mAccuracy << ", Time: " <<  std::asctime(std::localtime(&mTimeStamp));
	return ss.str();
}

} /* namespace CommNs */
