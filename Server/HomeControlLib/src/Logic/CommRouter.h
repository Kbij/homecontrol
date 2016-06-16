/*
 * HomeControl.h
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#ifndef LOGIC_HOMECONTROL_H_
#define LOGIC_HOMECONTROL_H_
#include "RoomListenerIf.h"
#include "TemperatureListenerIf.h"
#include "Comm/CommListenerIf.h"
#include <string>
#include <set>
#include <mutex>
#include <utility>
#include <list>
#include <set>

namespace CommNs
{
class CommServerIf;
class TemperatureSourceIf;
}
namespace DalNs
{
class HomeControlDalIf;
}
namespace LogicNs {
class RoomControl;


class CommRouter: public RoomListenerIf, public CommNs::CommListenerIf, public TemperatureListenerIf
{
public:
	CommRouter(DalNs::HomeControlDalIf* dal, CommNs::CommServerIf* server, CommNs::TemperatureSourceIf* sensors);
	virtual ~CommRouter();

	//RoomListenerIf
	void temperatureChanged(const std::string& roomId, double temperature);
	void setPointChanged(const std::string& roomId, double setTemperature);
	void heaterOn(const std::string& roomId);
	void heaterOff(const std::string& roomId);

	//CommListenerIf
	void clientConnected(const std::string& name);
	void clientDisConnected(const std::string& name);
	void receiveObject(const std::string name, const CommNs::CommObjectIf* object);

	//TemperatureListenerIf
	void sensorStarted(const std::string& sensorId);
	void sensorTemperature(const std::string& sensorId, double temperature);
	void sensorSetTemperatureUp(const std::string& sensorId);
	void sensorSetTemperatureDown(const std::string& sensorId);
private:
	DalNs::HomeControlDalIf* mDal;
	CommNs::CommServerIf* mCommServer;
	CommNs::TemperatureSourceIf* mSensors;
	std::set<std::string> mConnnectedClients;
	std::recursive_mutex mDataMutex;
	std::list<std::pair<std::set<std::string>, RoomControl*>> mRooms;

	RoomControl* findRoomByRoomId(const std::string& roomId, bool useDatabase);
	RoomControl* findRoomBySensorId(const std::string& sensorId);
	void addSensorToRoom(const std::string& roomId, const std::string& sensorId);
	void sendRooms();
};

} /* namespace LogicNs */

#endif /* LOGIC_HOMECONTROL_H_ */
