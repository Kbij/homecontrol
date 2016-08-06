/*
 * TemperatureSensors.cpp
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */

#include <Comm/TemperatureSensors.h>
#include "Comm/DMCommIf.h"
#include "Comm/DMMessages.h"
#include "Logic/TemperatureListenerIf.h"
#include <glog/logging.h>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>
namespace
{
const int MESSAGE_TYPE_POS = 0;
const int SERIAL_POS = 1;
const int TEMPERATURE_POS = 2;

const std::string MSG_SENSOR_STARTUP = "1";
const std::string MSG_TEMPERATURE = "2";
const std::string MSG_SERVER_DISCOVERY = "3";
const std::string MSG_SERVER_DISCOVERY_RESPONSE = "4";
const std::string MSG_SET_TEMPERATURE = "5";
const std::string MSG_SET_TEMPERATURE_UP = "6";
const std::string MSG_SET_TEMPERATURE_DOWN = "7";
}

namespace CommNs {

TemperatureSensors::TemperatureSensors(DMCommIf* dmComm):
	mDMComm(dmComm),
	mListeners(),
	mSensorAddress(),
	mDataMutex()
{
	if (mDMComm)
	{
		mDMComm->registerListener(this);
	}
}

TemperatureSensors::~TemperatureSensors()
{
	if (mDMComm)
	{
		mDMComm->unRegisterListener(this);
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

void TemperatureSensors::writeSetTemperature(const std::string& sensorId, double temperature)
{
	if (mDMComm)
	{
		std::stringstream ss;
		ss << "[" << MSG_SET_TEMPERATURE << ":"  << std::fixed << std::setprecision(1) << temperature << "]";
		std::string outputLine = ss.str();
		std::replace(outputLine.begin(), outputLine.end(), '.', ',');
		std::string dataString(ss.str());
		VLOG(1) << "Writing set temperature (" << temperature << ") to sensor: " << sensorId;

		if (mSensorAddress.find(sensorId) != mSensorAddress.end())
		{
			TxMessage* txMessage = new TxMessage(std::vector<uint8_t>(dataString.begin(), dataString.end()), mSensorAddress[sensorId]);
			mDMComm->sendMessage(txMessage);
		}
		else
		{
			LOG(ERROR) << "Unable to write set temperature, sensor unknown: " << sensorId;
		}
	}
}

void TemperatureSensors::receiveMessage(const DMMessageIf* message)
{
	if (message)
	{
		switch(message->messageType())
		{
			case DMMessageType::RxMessage:
			{
				if (const RxMessage* rxMessage = dynamic_cast<const RxMessage*>(message))
				{
					std::string line(rxMessage->rxMessage().begin(), rxMessage->rxMessage().end());
					receiveLine(line, rxMessage->sourceAddress());
				}
				break;
			}
			default:
				LOG(ERROR) << "Unhandled message type:" << message->toString();
				break;
		}
	}

}

void TemperatureSensors::receiveLine(const std::string& line, const std::vector<uint8_t> sourceAddress)
{
	try
	{
		if (line.size() > 2 && line.front() == '[' && line.back() == ']')
		{
			std::string rawLine(line.begin() + 1, line.end() -1);

			std::vector<std::string> lineParts;
			boost::split(lineParts, rawLine, boost::is_any_of(":"));
			if (lineParts.size() == 0)
			{
				LOG(ERROR) << "Invalid line: " << line;
				return;
			}

			if (lineParts[MESSAGE_TYPE_POS] == MSG_SENSOR_STARTUP)
			{
				std::string sensorId = lineParts[SERIAL_POS];
				mSensorAddress[sensorId] = sourceAddress;
				sendSensorStarted(sensorId);
				return;
			}
			if (lineParts[MESSAGE_TYPE_POS] == MSG_TEMPERATURE)
			{
				std::string sensorId = lineParts[SERIAL_POS];
				mSensorAddress[sensorId] = sourceAddress;
				float temp = std::stof(lineParts[TEMPERATURE_POS]);
				sendTemperature(sensorId, temp);
				return;
			}
			if (lineParts[MESSAGE_TYPE_POS] == MSG_SERVER_DISCOVERY)
			{
				sendXBeeListenerAddress(lineParts[SERIAL_POS], sourceAddress);
				return;
			}
			if (lineParts[MESSAGE_TYPE_POS] == MSG_SET_TEMPERATURE_UP)
			{
				std::lock_guard<std::mutex> lg(mDataMutex);
				for (auto listener: mListeners)
				{
					listener->sensorSetTemperatureUp(lineParts[SERIAL_POS]);
				}
				return;
			}
			if (lineParts[MESSAGE_TYPE_POS] == MSG_SET_TEMPERATURE_DOWN)
			{
				std::lock_guard<std::mutex> lg(mDataMutex);
				for (auto listener: mListeners)
				{
					listener->sensorSetTemperatureDown(lineParts[SERIAL_POS]);
				}
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

void TemperatureSensors::sendXBeeListenerAddress(const std::string& sensorId, const std::vector<uint8_t> sourceAddress)
{
	if (mDMComm)
	{
		LOG(INFO) << "Sensor (id: " << sensorId << ") does not know the server's address, sending now";
		std::string address(mDMComm->addressString());
		address.erase(std::remove(address.begin(), address.end(), ':'), address.end());
		//std::string address2 = std::remove(address.begin(), address.end(), ':');
		std::stringstream ss;
		ss << "[" << MSG_SERVER_DISCOVERY_RESPONSE << ":" << address << "]";
		std::string dataString(ss.str());

		TxMessage* txMessage = new TxMessage(std::vector<uint8_t>(dataString.begin(), dataString.end()), sourceAddress);
		mDMComm->sendMessage(txMessage);
	}


}
} /* namespace CommNs */
