/*
 * TemperatureSensors.h
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */

#ifndef COMM_TEMPERATURESENSORS_H_
#define COMM_TEMPERATURESENSORS_H_
#include <Comm/TemperatureSourceIf.h>
#include "SerialListenerIf.h"
#include <set>
#include <mutex>

namespace LogicNs
{
class TemperatureListenerIf;
}

namespace CommNs {
class SerialIf;

class TemperatureSensors: public SerialListenerIf, public TemperatureSourceIf
{
public:
	TemperatureSensors(SerialIf* serial);
	virtual ~TemperatureSensors();

	//SerialListenerIf
	void receiveLine(const std::string& line);
	void receiveData(const std::vector<uint8_t>& data) {};


	//TemperatureSensorsIf
	void registerTemperatureListener(LogicNs::TemperatureListenerIf* listener);
	void unRegisterTemperatureListener(LogicNs::TemperatureListenerIf* listener);

	void writeSetTemperature(const std::string& sensorId, double temperature);
private:
	SerialIf* mSerial;
	std::set<LogicNs::TemperatureListenerIf*> mListeners;
	std::mutex mDataMutex;

	void sendSensorStarted(const std::string& sensorId);
	void sendTemperature(const std::string& sensorId, float temperature);
	void sendSetTemperatureUp(const std::string& sensorId);
	void sendSetTemperatureDown(const std::string& sensorId);
};

} /* namespace CommNs */

#endif /* COMM_TEMPERATURESENSORS_H_ */
