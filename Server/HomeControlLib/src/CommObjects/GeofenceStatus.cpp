/*
 * GeofenceStatus.cpp
 *
 *  Created on: Sep 7, 2026
 *      Author: koen
 */

#include <CommObjects/GeofenceStatus.h>
#include <json/json.h>

namespace CommNs {

GeofenceStatus::GeofenceStatus(const std::string& json):
	mActive(false),
	mLat(0.0),
	mLon(0.0),
	mRadius(0.0)
{
	Json::Value jsonRoot;
	Json::Reader reader;
	if (reader.parse(json, jsonRoot, false))
	{
		if (jsonRoot.isMember("active"))
		{
			mActive = jsonRoot["active"].asBool();
		}
		if (jsonRoot.isMember("lat"))
		{
			mLat = jsonRoot["lat"].asDouble();
		}
		if (jsonRoot.isMember("lon"))
		{
			mLon = jsonRoot["lon"].asDouble();
		}
		if (jsonRoot.isMember("radius"))
		{
			mRadius = jsonRoot["radius"].asDouble();
		}
	}
}

GeofenceStatus::~GeofenceStatus()
{
}

uint8_t GeofenceStatus::objectId() const
{
	return 45;
}

std::string GeofenceStatus::toString() const
{
	return mActive ? "Geofence Status: active" : "Geofence Status: inactive";
}
} /* namespace CommNs */
