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

namespace DalNs
{
class HomeControlDalIf;
}
namespace LogicNs {

class HomeControl: public RoomListenerIf, public CommNs::CommListenerIf, public TemperatureListenerIf
{
public:
	HomeControl(DalNs::HomeControlDalIf* dal);
	virtual ~HomeControl();

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
	std::set<std::string> mConnnectedClients;
	std::mutex mDataMutex;
};

} /* namespace LogicNs */

#endif /* LOGIC_HOMECONTROL_H_ */
