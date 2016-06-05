/*
 * HomeControlDalIf.h
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#ifndef DAL_HOMECONTROLDALIF_H_
#define DAL_HOMECONTROLDALIF_H_
#include <string>

namespace DalNs {
struct RoomConfig
{
	std::string RoomId;
	std::string RoomName;
	std::vector<std::string> mSensorIds;
}

class HomeControlDalIf
{
public:
	virtual ~HomeControlDalIf() {};

	RoomConfig* findRoomByRoomId(const std::string& roomId);
	RoomConfig* findRoomBySensorId(const std::string& sensorId);
};

} /* namespace CommNs */



#endif /* DAL_HOMECONTROLDALIF_H_ */
