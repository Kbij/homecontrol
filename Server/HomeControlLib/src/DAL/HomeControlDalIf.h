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

};

} /* namespace DalNs */



#endif /* DAL_HOMECONTROLDALIF_H_ */
