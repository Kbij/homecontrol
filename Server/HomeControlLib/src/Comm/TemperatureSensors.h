/*
 * TemperatureSensors.h
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */

#ifndef COMM_TEMPERATURESENSORS_H_
#define COMM_TEMPERATURESENSORS_H_
#include <Comm/TemperatureSourceIf.h>
#include "Comm/DMCommListenerIf.h"
#include "Comm/TimeListenerIf.h"
#include <set>
#include <mutex>
#include <vector>
#include <map>
#include <string>

namespace LogicNs
{
class TemperatureListenerIf;
}

namespace CommNs {
class DMCommIf;

class TemperatureSensors: public DMCommListenerIf, public TemperatureSourceIf
{
public:
	TemperatureSensors(DMCommIf* dmComm);
	virtual ~TemperatureSensors();

	//DMCommListenerIf
	void receiveMessage(const DMMessageIf* message);

	//TemperatureSensorsIf
	void registerTemperatureListener(LogicNs::TemperatureListenerIf* listener);
	void unRegisterTemperatureListener(LogicNs::TemperatureListenerIf* listener);

	void writeSetTemperature(const std::string& sensorId, double temperature);
	void writeSensorConfig(const std::string& sensorId, double calibration, const std::string& roomName);

	//TimeListenerIf
	void writeTime(const std::string& time);

private:
	DMCommIf* mDMComm;
	std::set<LogicNs::TemperatureListenerIf*> mListeners;
	std::map<std::string, std::vector<uint8_t>> mSensorAddress;
	std::recursive_mutex mDataMutex;

	void receiveLine(const std::string& line, const std::vector<uint8_t> sourceAddress);
	void sendSensorStarted(const std::string& sensorId);
	void sendTemperature(const std::string& sensorId, float temperature);
	void sendSetTemperatureUp(const std::string& sensorId);
	void sendSetTemperatureDown(const std::string& sensorId);
	void sendXBeeListenerAddress(const std::string& sensorId, const std::vector<uint8_t> sourceAddress);
};

} /* namespace CommNs */

#endif /* COMM_TEMPERATURESENSORS_H_ */
