/*
 * TemperatureSensorsSimulator.h
 *
 *  Created on: Jun 10, 2016
 *      Author: koen
 */

#ifndef COMM_TEMPERATURESENSORSSIMULATOR_H_
#define COMM_TEMPERATURESENSORSSIMULATOR_H_
#include <Comm/TemperatureSourceIf.h>
#include <string>
#include <map>
#include <thread>
#include <utility>
#include <vector>
#include <set>

namespace CommNs {

class TemperatureSensorsSimulator: public TemperatureSourceIf
{
public:
	TemperatureSensorsSimulator(int sensorCount);
	virtual ~TemperatureSensorsSimulator();

	void registerTemperatureListener(LogicNs::TemperatureListenerIf* listener);
	void unRegisterTemperatureListener(LogicNs::TemperatureListenerIf* listener);

	void writeSetTemperature(const std::string& sensorId, double temperature);

	void writeSensorConfig(const std::string& sensorId, double calibration, const std::string& roomName) {};
	void writeTime(const std::string& time) {};
private:
	void startSimulationThread();
	void stopSimulationThread();
	void simulationThread();

	std::set<LogicNs::TemperatureListenerIf*> mListeners;
	std::vector<std::pair<std::string, double>> mSensors;
	bool mSimulationThreadRunning;
	std::thread* mSimulationThread;

};

} /* namespace CommNs */

#endif /* COMM_TEMPERATURESENSORSSIMULATOR_H_ */
