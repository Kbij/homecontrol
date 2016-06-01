/*
 * TemperatureSensors.cpp
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */

#include <Comm/TemperatureSensors.h>
#include "Comm/SerialIf.h"
#include "Logic/TemperatureListenerIf.h"
#include <glog/logging.h>
#include <boost/algorithm/string.hpp>

namespace
{
const int SERIAL_POS = 0;
const int MESSAGE_TYPE_POS = 1;
const int TEMPERATURE_POS = 2;
}

namespace CommNs {

TemperatureSensors::TemperatureSensors(SerialIf* serial):
	mSerial(serial),
	mListeners(),
	mDataMutex()
{
	if (mSerial)
	{
		mSerial->registerSerialListener(this);
	}
}

TemperatureSensors::~TemperatureSensors()
{
	if (mSerial)
	{
		mSerial->unRegisterSerialListener();
	}
}

void TemperatureSensors::registerTemperatureListener(LogicNs::TemperatureListenerIf* listener)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	mListeners.insert(listener);
}

void TemperatureSensors::unRegisterTemperatureListener(LogicNs::TemperatureListenerIf* listener)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	mListeners.erase(listener);
}

void TemperatureSensors::receiveLine(const std::string& line)
{
	try
	{
		if (line.size() > 2 && (*line.begin()) == '[' && (*(line.end()-1)) == ']')
		{
			std::string rawLine(line.begin() + 1, line.end() -1);

			std::vector<std::string> lineParts;
			boost::split(lineParts, rawLine, boost::is_any_of(":"));
			if (lineParts.size() == 0)
			{
				LOG(ERROR) << "Invalid line: " << line;
				return;
			}

			if (lineParts[MESSAGE_TYPE_POS] == "1")
			{
				sendSensorStarted(lineParts[SERIAL_POS]);
				return;
			}
			if (lineParts[MESSAGE_TYPE_POS] == "2")
			{
				float temp = std::stof(lineParts[TEMPERATURE_POS]);
				sendTemperature(lineParts[SERIAL_POS], temp);
				return;
			}
		}
		else
		{
			LOG(ERROR) << "Incomplete line received: " << line;
		}
	}
	catch(const std::exception& ex)
	{
		LOG(ERROR) << "Exception while received serial data: " << ex.what();
	}

}

void TemperatureSensors::sendSensorStarted(const std::string& sensorId)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	for (auto listener: mListeners)
	{
		listener->sensorStarted(sensorId);
	}
}

void TemperatureSensors::sendTemperature(const std::string& sensorId, float temperature)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	for (auto listener: mListeners)
	{
		listener->sensorTemperature(sensorId, temperature);
	}
}

void TemperatureSensors::sendSetTemperatureUp(const std::string& sensorId)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	for (auto listener: mListeners)
	{
		listener->sensorSetTemperatureUp(sensorId);
	}
}

void TemperatureSensors::sendSetTemperatureDown(const std::string& sensorId)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	for (auto listener: mListeners)
	{
		listener->sensorSetTemperatureDown(sensorId);
	}
}
} /* namespace CommNs */
