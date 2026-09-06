/*
 * LocationHistoryResponse.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#include <CommObjects/LocationHistoryResponse.h>
#include <json/json.h>

namespace CommNs {

LocationHistoryResponse::LocationHistoryResponse(const std::string& clientName):
	mClientName(clientName)
{
}

LocationHistoryResponse::~LocationHistoryResponse()
{
}

uint8_t LocationHistoryResponse::objectId() const
{
	return 44;
}

std::string LocationHistoryResponse::toString() const
{
	std::string result = "Location History Response, client: " + mClientName;
	return result;
}

std::string LocationHistoryResponse::json() const
{
	Json::Value root;
	root["client"] = mClientName;

	Json::Value pointArray;
	for(const auto& point: mPoints)
	{
		Json::Value jsonPoint;
		jsonPoint["lat"] = point.Latitude;
		jsonPoint["lon"] = point.Longitude;
		// Milliseconds since epoch, matching the millis convention the rest of the
		// protocol uses on the wire (see GpsLocationMessage on the Android side) - this
		// is a fresh response field, not the legacy .NET "/Date(...)/ " string format
		// GpsLocation itself has to parse for backward compatibility.
		jsonPoint["timestamp"] = static_cast<Json::Int64>(point.Timestamp) * 1000;
		pointArray.append(jsonPoint);
	}
	root["points"] = pointArray;

	Json::FastWriter writer;
	return writer.write(root);
}

void LocationHistoryResponse::addPoint(const LocationPoint& point)
{
	mPoints.push_back(point);
}
} /* namespace CommNs */
