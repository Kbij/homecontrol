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

namespace DalNs {
struct RoomConfig
{
	std::string RoomId;
	std::string RoomName;
	std::vector<std::string> mSensorIds;
};

class HomeControlDalIf
{
public:
	virtual ~HomeControlDalIf() {};

	virtual RoomConfig* findRoomByRoomId(const std::string& roomId) = 0;
	virtual RoomConfig* findRoomBySensorId(const std::string& sensorId) = 0;

	virtual double getSensorCalibration(const std::string& sensorId) = 0;
};

} /* namespace DalNs */



#endif /* DAL_HOMECONTROLDALIF_H_ */
