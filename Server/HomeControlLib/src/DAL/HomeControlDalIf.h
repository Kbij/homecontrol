/*
 * HomeControlDalIf.h
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#ifndef DAL_HOMECONTROLDALIF_H_
#define DAL_HOMECONTROLDALIF_H_
#include <string>
#include <vector>
#include <stdint.h>
#include <sstream>
#include <ctime>

namespace DalNs {
struct RoomConfig
{
	std::string RoomId;
	std::string RoomName;
	std::vector<std::string> SensorIds;
	uint8_t HeaterOutput;
	std::string toString()
	{
		std::stringstream ss;
		ss << "RoomId: " << RoomId << ", RoomName: " << RoomName << ", HeaterOutput: " << (int) HeaterOutput;
		return ss.str();
	}
};

// Kept separate from CommNs::LocationPoint (see CommObjects/LocationHistoryResponse.h) -
// same shape, but the DAL and protocol layers deliberately don't share types here (compare
// RoomConfig above vs. CommNs::Room).
struct LocationPoint
{
	double Latitude;
	double Longitude;
	time_t Timestamp;
};

// Kept separate from CommNs::GeofenceStatus/LocationHistoryResponse's own geofence fields,
// same convention as LocationPoint above. CreatedAt/UpdatedAt are both 0 when !Active.
struct GeofenceInfo
{
	bool Active;
	double Latitude;
	double Longitude;
	double RadiusMeters;
	time_t CreatedAt;
	time_t UpdatedAt;
};

class HomeControlDalIf
{
public:
	virtual ~HomeControlDalIf() {};

	virtual RoomConfig* findRoomByRoomId(const std::string& roomId) = 0;
	virtual RoomConfig* findRoomBySensorId(const std::string& sensorId) = 0;

	virtual double getSensorCalibration(const std::string& sensorId) = 0;

	virtual int locationInterval(const std::string& clientId) = 0;
	virtual void writeHeaterOn(const std::string& roomId) = 0;
	virtual void writeHeaterOff(const std::string& roomId) = 0;

	virtual int findDevice(const std::string& device) = 0;
	virtual void logLocation(int deviceId, double lat, double lon, double accuracy, double batteryLevel, time_t timestamp) = 0;

	// Admin mode (see Logic/AdminController): empty string means the client has no admin
	// PIN configured, i.e. admin mode is unavailable for it.
	virtual std::string adminCode(const std::string& clientId) = 0;
	virtual std::vector<std::string> allClientNames() = 0;
	virtual std::vector<LocationPoint> locationHistory(const std::string& clientId, int minutes) = 0;

	// Geofencing (see Logic/AdminController, CommObjects/GeofenceStatus). The client always
	// reports its *current* fence unconditionally; updateGeofence() itself decides create vs.
	// renew (whether CreatedAt already has a value), so there's no separate "create" call.
	virtual void updateGeofence(const std::string& clientId, double lat, double lon, double radiusMeters) = 0;
	virtual void clearGeofence(const std::string& clientId) = 0;
	virtual GeofenceInfo geofence(const std::string& clientId) = 0;

};

} /* namespace DalNs */



#endif /* DAL_HOMECONTROLDALIF_H_ */
