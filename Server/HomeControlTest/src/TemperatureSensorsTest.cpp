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
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>

class TemperatureSensorStub: public LogicNs::TemperatureListenerIf
{
public:
	TemperatureSensorStub(): mLastId(), mLastTemperature() {};
	~TemperatureSensorStub() {};

	void sensorStarted(const std::string& sensorId)
	{
		mLastId = sensorId;
	}
	void sensorTemperature(const std::string& sensorId, float temperature)
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
	float mLastTemperature;
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
