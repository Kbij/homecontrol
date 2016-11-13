/*
 * HomeControlDal.h
 *
 *  Created on: Jun 10, 2016
 *      Author: koen
 */

#ifndef DAL_HOMECONTROLDAL_H_
#define DAL_HOMECONTROLDAL_H_
#include "HomeControlDalIf.h"
#include <string>

namespace DalNs {

class HomeControlDal: public HomeControlDalIf
{
public:
	HomeControlDal();
	virtual ~HomeControlDal();

	RoomConfig* findRoomByRoomId(const std::string& roomId);
	RoomConfig* findRoomBySensorId(const std::string& sensorId);

	double getSensorCalibration(const std::string& sensorId);
};

} /* namespace DalNs */

#endif /* DAL_HOMECONTROLDAL_H_ */
