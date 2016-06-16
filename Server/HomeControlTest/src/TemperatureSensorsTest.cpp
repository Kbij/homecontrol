/*
 * TemperatureSensorsTest.cpp
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */


#include "Comm/SerialIf.h"
#include "Comm/SerialListenerIf.h"
#include "Comm/TemperatureSensors.h"
#include "Logic/TemperatureListenerIf.h"
#include "Logic/TemperatureFilter.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <algorithm>
#include <string>
#include <thread>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <sstream>

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

	void writeLine(const std::string& line)
	{
		mLastSend = line;
	}
	std::string mLastSend;
};

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

	sensors->receiveLine("[28AC48FB07000077:1");
	EXPECT_EQ("", tempListenerStub.mLastId);

	sensors->receiveLine("[28AC48FB07000077:1");
	EXPECT_EQ("", tempListenerStub.mLastId);

	sensors->receiveLine("28AC48FB07000077:1]");
	EXPECT_EQ("", tempListenerStub.mLastId);

	sensors->receiveLine("[]");
	EXPECT_EQ("", tempListenerStub.mLastId);


	sensors->receiveLine("[28AC48FB07000077]");
	EXPECT_EQ("", tempListenerStub.mLastId);

	sensors->unRegisterTemperatureListener(&tempListenerStub);
	delete sensors;
}

TEST(TemperatureSensors, ReceiveValidStartup)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureListenerStub tempListenerStub;
	sensors->registerTemperatureListener(&tempListenerStub);

	sensors->receiveLine("[28AC48FB07000077:1]");
	EXPECT_EQ("28AC48FB07000077", tempListenerStub.mLastId);

	sensors->unRegisterTemperatureListener(&tempListenerStub);
	delete sensors;
}

TEST(TemperatureSensors, ReceiveValidTemperature)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureListenerStub tempListenerStub;
	sensors->registerTemperatureListener(&tempListenerStub);

	sensors->receiveLine("[28AC48FB07000077:2:23.75]");
	EXPECT_EQ("28AC48FB07000077", tempListenerStub.mLastId);
	EXPECT_EQ(23.75, tempListenerStub.mLastTemperature);

	sensors->receiveLine("[28AC48FB07000000:2:23,75]");
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

	sensors->receiveLine("[28AC48FB07000077:2:/25/75]");
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

TEST(TemperatureSensors, SendSetTemperature)
{
	SerialStub serialStub;
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(&serialStub);

	sensors->writeSetTemperature("mySensor", 25.6);
	EXPECT_EQ(serialStub.mLastSend, "[mySensor:5:25,60]");
	delete sensors;
}
