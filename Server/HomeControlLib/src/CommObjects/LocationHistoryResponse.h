/*
 * LocationHistoryResponse.h
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#ifndef COMMOBJECTS_LOCATIONHISTORYRESPONSE_H_
#define COMMOBJECTS_LOCATIONHISTORYRESPONSE_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>
#include <vector>
#include <ctime>

namespace CommNs {

/**
 * Deliberately separate from DalNs::LocationPoint (see HomeControlDalIf.h) - same shape,
 * but keeping the protocol (CommObjects) layer decoupled from the DAL layer's types is
 * the existing convention here (compare CommNs::Room vs. DalNs::RoomConfig).
 */
struct LocationPoint
{
	double Latitude;
	double Longitude;
	time_t Timestamp;
};

/**
 * Server -> client, objectId 44. Reply to a LocationHistoryRequest, containing every
 * point logged for the requested client within the requested window, oldest first.
 */
class LocationHistoryResponse: public CommObjectIf
{
public:
	LocationHistoryResponse(const std::string& clientName);
	virtual ~LocationHistoryResponse();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;

	void addPoint(const LocationPoint& point);

	/** Attaches the client's current geofence (see DalNs::GeofenceInfo), if any. Not called
	 *  at all (or called with active=false) means the "geofence" field is simply omitted
	 *  from json() - the admin map only draws a circle when this was set with active=true. */
	void setGeofence(bool active, double lat, double lon, double radiusMeters, time_t createdAt, time_t updatedAt);

	/** Sets Client.lastMessage - when the server last heard *anything* from this client
	 *  (including keepalives), as opposed to the timestamp of its last *location* point above.
	 *  Always included in json() (0 = unknown), unlike the geofence field which is conditional. */
	void setLastConnection(time_t lastConnection);

private:
	std::string mClientName;
	std::vector<LocationPoint> mPoints;
	bool mGeofenceActive;
	double mGeofenceLat;
	double mGeofenceLon;
	double mGeofenceRadiusMeters;
	time_t mGeofenceCreatedAt;
	time_t mGeofenceUpdatedAt;
	time_t mLastConnection;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_LOCATIONHISTORYRESPONSE_H_ */
