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
	mClientName(clientName),
	mGeofenceActive(false),
	mGeofenceLat(0.0),
	mGeofenceLon(0.0),
	mGeofenceRadiusMeters(0.0),
	mGeofenceCreatedAt(0),
	mGeofenceUpdatedAt(0)
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

	if (mGeofenceActive)
	{
		Json::Value geofence;
		geofence["lat"] = mGeofenceLat;
		geofence["lon"] = mGeofenceLon;
		geofence["radius"] = mGeofenceRadiusMeters;
		geofence["createdAt"] = static_cast<Json::Int64>(mGeofenceCreatedAt) * 1000;
		geofence["updatedAt"] = static_cast<Json::Int64>(mGeofenceUpdatedAt) * 1000;
		root["geofence"] = geofence;
	}

	Json::FastWriter writer;
	return writer.write(root);
}

void LocationHistoryResponse::addPoint(const LocationPoint& point)
{
	mPoints.push_back(point);
}

void LocationHistoryResponse::setGeofence(bool active, double lat, double lon, double radiusMeters, time_t createdAt, time_t updatedAt)
{
	mGeofenceActive = active;
	mGeofenceLat = lat;
	mGeofenceLon = lon;
	mGeofenceRadiusMeters = radiusMeters;
	mGeofenceCreatedAt = createdAt;
	mGeofenceUpdatedAt = updatedAt;
}
} /* namespace CommNs */
