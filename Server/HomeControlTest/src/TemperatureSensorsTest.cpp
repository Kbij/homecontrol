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

class TemperatureSensorStub: public LogicNs::TemperatureListenerIf
{
public:
	TemperatureSensorStub(): mLastId(), mLastTemperature() {};
	~TemperatureSensorStub() {};

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

TEST(TemperatureSensors, Constructor)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureSensorStub tempSensorStub;
	sensors->registerTemperatureListener(&tempSensorStub);


	sensors->unRegisterTemperatureListener(&tempSensorStub);
	delete sensors;
}

TEST(TemperatureSensors, ReceiveInvalid)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureSensorStub tempSensorStub;
	sensors->registerTemperatureListener(&tempSensorStub);

	sensors->receiveLine("[28AC48FB07000077:1");
	EXPECT_EQ("", tempSensorStub.mLastId);

	sensors->receiveLine("[28AC48FB07000077:1");
	EXPECT_EQ("", tempSensorStub.mLastId);

	sensors->receiveLine("28AC48FB07000077:1]");
	EXPECT_EQ("", tempSensorStub.mLastId);

	sensors->receiveLine("[]");
	EXPECT_EQ("", tempSensorStub.mLastId);


	sensors->receiveLine("[28AC48FB07000077]");
	EXPECT_EQ("", tempSensorStub.mLastId);

	sensors->unRegisterTemperatureListener(&tempSensorStub);
	delete sensors;
}

TEST(TemperatureSensors, ReceiveValidStartup)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureSensorStub tempSensorStub;
	sensors->registerTemperatureListener(&tempSensorStub);

	sensors->receiveLine("[28AC48FB07000077:1]");
	EXPECT_EQ("28AC48FB07000077", tempSensorStub.mLastId);

	sensors->unRegisterTemperatureListener(&tempSensorStub);
	delete sensors;
}

TEST(TemperatureSensors, ReceiveValidTemperature)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureSensorStub tempSensorStub;
	sensors->registerTemperatureListener(&tempSensorStub);

	sensors->receiveLine("[28AC48FB07000077:2:23.75]");
	EXPECT_EQ("28AC48FB07000077", tempSensorStub.mLastId);
	EXPECT_EQ(23.75, tempSensorStub.mLastTemperature);

	sensors->receiveLine("[28AC48FB07000000:2:23,75]");
	EXPECT_EQ("28AC48FB07000000", tempSensorStub.mLastId);
	EXPECT_EQ(23.0, tempSensorStub.mLastTemperature);

	sensors->unRegisterTemperatureListener(&tempSensorStub);
	delete sensors;
}

TEST(TemperatureSensors, ReceiveInvalidTemperature)
{
	CommNs::TemperatureSensors* sensors = new CommNs::TemperatureSensors(nullptr);
	TemperatureSensorStub tempSensorStub;
	sensors->registerTemperatureListener(&tempSensorStub);

	sensors->receiveLine("[28AC48FB07000077:2:/25/75]");
	EXPECT_EQ("", tempSensorStub.mLastId);
	EXPECT_EQ(0, tempSensorStub.mLastTemperature);

	sensors->unRegisterTemperatureListener(&tempSensorStub);
	delete sensors;
}

void filter(const std::string& inputFile, const std::string& outputFile)
{
	TemperatureSensorStub tempSensorStub;
	LogicNs::TemperatureFilter* filter = new LogicNs::TemperatureFilter(&tempSensorStub);
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
		std::cout << line << std::endl;
		try
		{
			std::string tempString = lineParts[2];
			std::replace(tempString.begin(), tempString.end(), ',', '.');
			float tempRead = std::stof(tempString);
			filter->sensorTemperature("MYSENSOR", tempRead);
			ofFile << lineParts[0] << ";" << lineParts[1] << ";" << tempRead << ";" << tempSensorStub.mLastTemperature << "\n";
		}
		catch (...) {};
	}
	ofFile.close();
	delete filter;
}

TEST(TemperatureSensors, Filter)
{
	filter("../testdata/LivingTemperatuur.csv", "Processed.csv");
	filter("../testdata/LivingTemperatuur2.csv", "Processed2.csv");
}
