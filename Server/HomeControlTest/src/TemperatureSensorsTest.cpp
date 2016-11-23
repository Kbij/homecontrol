/*
 * TemperatureSensorsTest.cpp
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */


#include "Comm/SerialIf.h"
#include "Comm/SerialListenerIf.h"
#include "Comm/TemperatureSensors.h"
#include "Comm/DMMessages.h"
#include "Comm/DMCommIf.h"
#include "Logic/TemperatureListenerIf.h"
#include "Logic/TemperatureFilter.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <algorithm>
#include <vector>
#include <stdint.h>
#include <string>
#include <thread>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <sstream>

namespace CommNs
{
class DMCommListenerIf;
}

namespace
{
class TemperatureListenerStub: public LogicNs::TemperatureListenerIf
{
public:
	TemperatureListenerStub(): mLastId(), mLastTemperature() {};
	~TemperatureListenerStub() {};

	void sensorStarted(const std::string& sensorId)
	{
		mLastId = sensorId;
	}
	void sensorTemperature(const std::string& sensorId, double temperature)
	{
		//LOG(INFO) << "Temperature, sensor: " << sensorId << "Temp: " << temperature;
		mLastId = sensorId;
		mLastTemperature = temperature;
	}
	void sensorSetTemperatureUp(const std::string& sensorId)
	{
		mLastId = sensorId;
	}
	void sensorSetTemperatureDown(const std::string& sensorId)
	{
		mLastId = sensorId;
	}
	std::string mLastId;
	double mLastTemperature;
};

class SerialStub: public CommNs::SerialIf
{
public:
	SerialStub(): mLastSend() {};
	virtual ~SerialStub() {};

	void registerSerialListener(CommNs::SerialListenerIf* listener) {};
	void unRegisterSerialListener() {};

	void writeData(const std::vector<uint8_t>& data)
	{
		mLastSend = data;
	};
	std::vector<uint8_t> mLastSend;
};

class DMCommStub:public CommNs::DMCommIf
{
public:
	DMCommStub():
	 mAddress(),
	 mMessage(nullptr)
	 {
	 };
	virtual ~DMCommStub()
	{
		if (mMessage)
		{
			delete mMessage;
		}
	};
	void registerListener(CommNs::DMCommListenerIf* listener) {};
	void unRegisterListener(CommNs::DMCommListenerIf* listener) {};

	std::string addressString() {return mAddress; };

	void sendATCmd(const std::string& atCmd, std::vector<uint8_t> parameters) {};;
	CommNs::DMMessageIf* sendATCmd(const std::string& atCmd, std::vector<uint8_t> parameters, int timeOutMilliseconds) {return nullptr;};
	void sendMessage(CommNs::DMMessageIf* message)
	{
		if (mMessage)
		{
			delete mMessage;
		}
		mMessage = message;
	}
	CommNs::DMMessageIf* sendMessage(CommNs::DMMessageIf* message, int /*timeout*/)
	{
		if (mMessage)
		{
			delete mMessage;
		}
		mMessage = message;
		return nullptr;
	}

	void init() {};
	std::string mAddress;
	CommNs::DMMessageIf* mMessage;
};
}

TEST(TemperatureSensors, Constructor)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureListenerStub tempListenerStub;
	sensors->registerTemperatureListener(&tempListenerStub);


	sensors->unRegisterTemperatureListener(&tempListenerStub);
	delete sensors;
}

TEST(TemperatureSensors, ReceiveInvalid)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureListenerStub tempListenerStub;
	sensors->registerTemperatureListener(&tempListenerStub);
	std::vector<uint8_t> headerData(0, 12);

	std::vector<uint8_t> data1(12, 0);
	std::string string1 = "[1:28AC48FB07000077";
	data1.insert(data1.end(), string1.begin(), string1.end());
	CommNs::RxMessage message1(data1);
	sensors->receiveMessage(&message1);
	EXPECT_EQ("", tempListenerStub.mLastId);

	std::vector<uint8_t> data2(12, 0);
	std::string string2 = "[1:28AC48FB07000077";
	data2.insert(data2.end(), string2.begin(), string2.end());
	CommNs::RxMessage message2(data2);
	sensors->receiveMessage(&message2);
	EXPECT_EQ("", tempListenerStub.mLastId);

	std::vector<uint8_t> data3(12, 0);
	std::string string3 = "1:28AC48FB07000077]";
	data3.insert(data3.end(), string3.begin(), string3.end());
	CommNs::RxMessage message3(data3);
	sensors->receiveMessage(&message3);
	//sensors->receiveLine("1:28AC48FB07000077]");
	EXPECT_EQ("", tempListenerStub.mLastId);

	std::vector<uint8_t> data4(12, 0);
	std::string string4 = "[]";
	data4.insert(data4.end(), string4.begin(), string4.end());
	CommNs::RxMessage message4(data4);
	sensors->receiveMessage(&message4);
	EXPECT_EQ("", tempListenerStub.mLastId);

	std::vector<uint8_t> data5(12, 0);
	std::string string5 = "[28AC48FB07000077]";
	data5.insert(data5.end(), string5.begin(), string5.end());
	CommNs::RxMessage message5(data5);
	sensors->receiveMessage(&message5);
	EXPECT_EQ("", tempListenerStub.mLastId);

	sensors->unRegisterTemperatureListener(&tempListenerStub);
	delete sensors;
}

TEST(TemperatureSensors, ReceiveValidStartup)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureListenerStub tempListenerStub;
	sensors->registerTemperatureListener(&tempListenerStub);

	std::vector<uint8_t> data1(12, 0);
	std::string string1 = "[1:28AC48FB07000077]";
	data1.insert(data1.end(), string1.begin(), string1.end());
	CommNs::RxMessage message1(data1);

	sensors->receiveMessage(&message1);

	EXPECT_EQ("28AC48FB07000077", tempListenerStub.mLastId);

	sensors->unRegisterTemperatureListener(&tempListenerStub);
	delete sensors;
}

TEST(TemperatureSensors, ReceiveValidTemperature)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureListenerStub tempListenerStub;
	sensors->registerTemperatureListener(&tempListenerStub);

	std::vector<uint8_t> data1(12, 0);
	std::string string1 = "[2:28AC48FB07000077:23.75]";
	data1.insert(data1.end(), string1.begin(), string1.end());
	CommNs::RxMessage message1(data1);
	sensors->receiveMessage(&message1);
	//sensors->receiveLine("[2:28AC48FB07000077:23.75]");
	EXPECT_EQ("28AC48FB07000077", tempListenerStub.mLastId);
	EXPECT_EQ(23.75, tempListenerStub.mLastTemperature);


	std::vector<uint8_t> data2(12, 0);
	std::string string2 = "[2:28AC48FB07000000:23,75]";
	data2.insert(data2.end(), string2.begin(), string2.end());
	CommNs::RxMessage message2(data2);
	sensors->receiveMessage(&message2);
	EXPECT_EQ("28AC48FB07000000", tempListenerStub.mLastId);
	EXPECT_EQ(23.0, tempListenerStub.mLastTemperature);

	sensors->unRegisterTemperatureListener(&tempListenerStub);
	delete sensors;
}

TEST(TemperatureSensors, ReceiveInvalidTemperature)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureListenerStub tempListenerStub;
	sensors->registerTemperatureListener(&tempListenerStub);

	std::vector<uint8_t> data1(12, 0);
	std::string string1 = "[2:28AC48FB07000077:/25/75]";
	data1.insert(data1.end(), string1.begin(), string1.end());
	CommNs::RxMessage message1(data1);
	sensors->receiveMessage(&message1);
	EXPECT_EQ("", tempListenerStub.mLastId);
	EXPECT_EQ(0, tempListenerStub.mLastTemperature);

	sensors->unRegisterTemperatureListener(&tempListenerStub);
	delete sensors;
}

void filter(const std::string& inputFile, const std::string& outputFile, int k)
{
	TemperatureListenerStub tempListenerStub;
	LogicNs::TemperatureFilter* filter = new LogicNs::TemperatureFilter(nullptr, k);
	filter->registerTemperatureListener(&tempListenerStub);
	std::remove(outputFile.c_str());
	std::ifstream inFile(inputFile);
	std::ofstream ofFile(outputFile);
	std::string line;
	while (std::getline(inFile, line))
	{
		std::vector<std::string> lineParts;
		boost::split(lineParts, line, boost::is_any_of(";"));
		if (lineParts.size() < 2 )
		{
			LOG(ERROR) << "Invalid line: " << line;
			continue;
		}

		try
		{
			std::string tempString = lineParts[3];
			std::replace(tempString.begin(), tempString.end(), ',', '.');
			float tempRead = std::stof(tempString);
			filter->sensorTemperature("MYSENSOR", tempRead);

			if (tempListenerStub.mLastTemperature)
			{

				std::stringstream ss;
				ss << lineParts[0] << ";" << lineParts[2] << ";" << tempRead << ";" << tempListenerStub.mLastTemperature << "\n";
				std::string outputLine = ss.str();
				std::replace(outputLine.begin(), outputLine.end(), '.', ',');
				ofFile << outputLine;
			}
		}
		catch (...) {};
	}
	ofFile.close();

	filter->unRegisterTemperatureListener(&tempListenerStub);
	delete filter;
}
/*
TEST(TemperatureSensors, Filter)
{
	filter("../testdata/LivingTemperatuur.csv", "Processed_3.csv", 3);
	filter("../testdata/LivingTemperatuur.csv", "Processed_4.csv", 4);
	filter("../testdata/LivingTemperatuur.csv", "Processed_5.csv", 5);
	filter("../testdata/LivingTemperatuur.csv", "Processed_6.csv", 6);
	filter("../testdata/LivingTemperatuur.csv", "Processed_10.csv", 10);
	filter("../testdata/LivingTemperatuur.csv", "Processed_15.csv", 15);

//	filter("../testdata/LivingTemperatuur2.csv", "Processed2.csv");
}
*/

void filter2(const std::string& inputFile, const std::string& outputFile, double k)
{
	TemperatureListenerStub tempListenerStub;
	LogicNs::TemperatureFilter* filter = new LogicNs::TemperatureFilter(nullptr, k);
	filter->registerTemperatureListener(&tempListenerStub);
	std::remove(outputFile.c_str());
	std::ifstream inFile(inputFile);
	std::ofstream ofFile(outputFile);
	std::string line;
	while (std::getline(inFile, line))
	{
		try
		{
			std::replace(line.begin(), line.end(), ',', '.');
			float tempRead = std::stof(line);
			filter->sensorTemperature("MYSENSOR", tempRead);
			std::stringstream ss;
			ss << tempListenerStub.mLastTemperature << "\n";
			std::string outputLine = ss.str();
			std::replace(outputLine.begin(), outputLine.end(), '.', ',');
			ofFile << outputLine;
		}
		catch (...) {};
	}
	ofFile.close();

	filter->unRegisterTemperatureListener(&tempListenerStub);
	delete filter;
}

TEST(TemperatureSensors, Filter2)
{
	filter2("Unfiltered.csv", "Processed_0-2.csv", 0.2);
//	filter2("Unfiltered.csv", "Processed_0-9.csv", 0.9);
}

TEST(TemperatureSensors, SendSetTemperature)
{
//	SerialStub serialStub;
//	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(&serialStub);
//
//	sensors->writeSetTemperature("mySensor", 25.6);
//	//EXPECT_EQ(serialStub.mLastSend, "[mySensor:5:25,60]");
//	delete sensors;
}

TEST(TemperatureSensors, SendListenerAddress)
{
	DMCommStub* commStub = new  DMCommStub;
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(commStub);
	TemperatureListenerStub tempListenerStub;
	sensors->registerTemperatureListener(&tempListenerStub);

	EXPECT_EQ(nullptr, commStub->mMessage);
	std::vector<uint8_t> data1;
	data1.push_back(0);
	data1.push_back(0x01); //Address of sender
	data1.push_back(0x02);
	data1.push_back(0x03);
	data1.push_back(0x04);
	data1.push_back(0x05);
	data1.push_back(0x06);
	data1.push_back(0x07);
	data1.push_back(0x08);
	data1.push_back(0);
	data1.push_back(0);
	data1.push_back(0);
	std::string string1 = "[3:28AC48FB07000077]";
	data1.insert(data1.end(), string1.begin(), string1.end());
	CommNs::RxMessage message1(data1);
	sensors->receiveMessage(&message1);

	EXPECT_NE(nullptr, commStub->mMessage);
	sensors->unRegisterTemperatureListener(&tempListenerStub);
	delete sensors;
	delete commStub;
}
