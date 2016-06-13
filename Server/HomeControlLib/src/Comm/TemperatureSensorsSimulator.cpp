/*
 * TemperatureSensorsSimulator.cpp
 *
 *  Created on: Jun 10, 2016
 *      Author: koen
 */

#include <Comm/TemperatureSensorsSimulator.h>
#include "Logic/TemperatureListenerIf.h"
#include <sstream>
#include <glog/logging.h>

namespace CommNs {

TemperatureSensorsSimulator::TemperatureSensorsSimulator(int sensorCount):
		mListeners(),
	mSensors(),
	mSimulationThreadRunning(false),
	mSimulationThread(nullptr)
{
	LOG(INFO) << "Sensor simulator, number of sensors: " << sensorCount;
	for(int i=0; i < sensorCount; ++i)
	{
		std::stringstream ss;
		ss << "sensor" << i;
		mSensors.push_back(std::make_pair(ss.str(), 20));
	}
	startSimulationThread();
}

TemperatureSensorsSimulator::~TemperatureSensorsSimulator()
{
	LOG(INFO) << "Stop simulator";
	stopSimulationThread();
}

void TemperatureSensorsSimulator::registerTemperatureListener(LogicNs::TemperatureListenerIf* listener)
{
	mListeners.insert(listener);
}

void TemperatureSensorsSimulator::unRegisterTemperatureListener(LogicNs::TemperatureListenerIf* listener)
{
	mListeners.erase(listener);
}

void TemperatureSensorsSimulator::writeSetTemperature(const std::string& sensorId, double temperature)
{

}



void TemperatureSensorsSimulator::startSimulationThread()
{
	if (!mSimulationThreadRunning)
	{
		mSimulationThreadRunning = true;
		mSimulationThread = new std::thread(&TemperatureSensorsSimulator::simulationThread, this);
	}
}

void TemperatureSensorsSimulator::stopSimulationThread()
{
	mSimulationThreadRunning = false;

    if (mSimulationThread)
    {
    	mSimulationThread->join();
    	delete mSimulationThread;
    	mSimulationThread = nullptr;
    }
}

void TemperatureSensorsSimulator::simulationThread()
{
	int i = 0;
	while(mSimulationThreadRunning)
	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		if (mSensors.size() > 0)
		{
			if ((size_t) i >= mSensors.size())
			{
				i = 0;
			}
			mSensors[i].second += 0.2;
			LOG(INFO) << "Sensor: " << mSensors[i].first <<", temperature: " << mSensors[i].second;
			if (mSensors[i].second > 27)
			{
				mSensors[i].second = 17;
			}

			for(auto& listener: mListeners)
			{
				listener->sensorTemperature(mSensors[i].first, mSensors[i].second);
			}
			++i;
		}
	}
	LOG(INFO) << "Stimulation thread done";
}

} /* namespace CommNs */
