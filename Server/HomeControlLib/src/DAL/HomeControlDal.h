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
#include <map>

namespace DalNs {

class HomeControlDal: public HomeControlDalIf
{
public:
	HomeControlDal(const std::string& server, const std::string& user, const std::string& pwd);
	virtual ~HomeControlDal();

	RoomConfig* findRoomByRoomId(const std::string& roomId);
	RoomConfig* findRoomBySensorId(const std::string& sensorId);

	double getSensorCalibration(const std::string& sensorId);
	int locationInterval(const std::string& clientId);
	void writeHeaterOn(const std::string& roomId);
	void writeHeaterOff(const std::string& roomId);

private:
	void writeHeaterState(const std::string& roomId, bool state);
	const std::string mServer;
	const std::string mUser;
	const std::string mPwd;
	std::map<std::string, bool> mHeaterState;

};

} /* namespace DalNs */

#endif /* DAL_HOMECONTROLDAL_H_ */
