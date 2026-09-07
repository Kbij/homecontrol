/*
 * GeofenceStatus.h
 *
 *  Created on: Sep 7, 2026
 *      Author: koen
 */

#ifndef COMMOBJECTS_GEOFENCESTATUS_H_
#define COMMOBJECTS_GEOFENCESTATUS_H_
#include "CommObjectIf.h"
#include <string>

namespace CommNs {

/**
 * Client -> server, objectId 45. Payload: JSON {"active": bool, "lat": double, "lon":
 * double, "radius": double (meters)}. Sent by LocationLoggingService whenever a native
 * Android geofence is registered (active=true, with the fence's fixed center/radius),
 * re-confirmed (active=true, same center/radius, sent again by the 30-minute safety-net
 * check or an Android DWELL callback), or left (active=false - lat/lon/radius are unset/0
 * and ignored in that case).
 *
 * Whether this is a *create* or a *renew* is decided server-side (see
 * HomeControlDalIf::updateGeofence): the client always reports its current fence
 * unconditionally rather than tracking "have I already told the server about this one"
 * itself - simpler, and self-correcting if a report is ever lost.
 */
class GeofenceStatus: public CommObjectIf
{
public:
	GeofenceStatus(const std::string& json);
	virtual ~GeofenceStatus();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const {return "";};

	bool active() const {return mActive;};
	double lat() const {return mLat;};
	double lon() const {return mLon;};
	double radius() const {return mRadius;};
private:
	bool mActive;
	double mLat;
	double mLon;
	double mRadius;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_GEOFENCESTATUS_H_ */
