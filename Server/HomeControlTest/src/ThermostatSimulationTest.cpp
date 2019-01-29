/*
 * ThermostatSimulationTest.cpp
 *
 *  Created on: Jan 27, 2019
 *      Author: koen
 */

#include "Logic/TwoPointThermostat.h"
#include "Logic/ThermostatListenerIf.h"
#include "Logic/ThermostatIf.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <fstream>
/*
 *
 */
class ThermostatSimulation : public ::testing::Test, public LogicNs::ThermostatListenerIf
{
protected:
	ThermostatSimulation() :
		mThermo(),
		mRoomTemperature(16),
		mSetTemperature(16),
		mTempChangePerSecond(4.2/(270.0 * 60.0)),
		mHeaterState(false),
		mCurrentTime(),
		mPreviousTime(),
		mHeaterOnTime(-10000), // Very long time ago
		mHeaterOffTime(-10000), // Very long time ago
		mHeaterOnDelaySeconds(660),
		mHeaterOffDelaySeconds(2400)
	{
	}

	void SetUp() override
	{

	}

	void TearDown() override
	{

	}

	void heaterOn()
	{
		if (!mHeaterState)
		{
			LOG(INFO) << "Heater on, time: " <<  mCurrentTime;
			mHeaterOnTime = mCurrentTime;
			mHeaterState = true;
		}
	}

	void heaterOff()
	{
		if (mHeaterState)
		{
			LOG(INFO) << "Heater off, time: " <<  mCurrentTime;
			mHeaterOffTime = mCurrentTime;
			mHeaterState = false;
		}
	}

	void clear()
	{
	}

	void process(time_t time)
	{
		mCurrentTime = time;
		bool increaseTemp = false;
		if (mHeaterState)
		{ //Heater is on
			increaseTemp = mCurrentTime > (mHeaterOnTime + mHeaterOnDelaySeconds) ? true : false;
		}
		else
		{
			increaseTemp =  mCurrentTime > (mHeaterOffTime + mHeaterOffDelaySeconds) ? false : true;
		}
		// increase or decrease step * seconds passed
		double temperatureSep = mTempChangePerSecond * (mCurrentTime - mPreviousTime);
		mRoomTemperature += increaseTemp ?  temperatureSep : -temperatureSep;
		LOG(INFO) << "Time: " << mCurrentTime << ", Increase: " << increaseTemp  << ", step: " << temperatureSep;

		mPreviousTime = mCurrentTime;
		LOG(INFO) << "Temp: " << mRoomTemperature;
		if (mThermo) mThermo->temperatureChanged(time, mRoomTemperature);
	}
	LogicNs::ThermostatIf* mThermo;
	double mRoomTemperature;
	double mSetTemperature;
	double mTempChangePerSecond;

	bool mHeaterState;
	time_t mCurrentTime;
	time_t mPreviousTime;
	time_t mHeaterOnTime;
	time_t mHeaterOffTime;
	time_t mHeaterOnDelaySeconds;
	time_t mHeaterOffDelaySeconds;

};

class ThermoListenerStub: public LogicNs::ThermostatListenerIf
{
public:
	ThermoListenerStub(): mHeaterOnCalled(), mHeaterOffCalled() {};
	virtual ~ThermoListenerStub(){};
	void heaterOn() {mHeaterOnCalled = true;};
	void heaterOff() {mHeaterOffCalled = true;};
	void clear() {mHeaterOnCalled = false; mHeaterOffCalled = false;};
	bool mHeaterOnCalled;
	bool mHeaterOffCalled;
};

TEST_F(ThermostatSimulation, TwopointThermo)
{
	mThermo = new LogicNs::TwoPointThermostat(0.5);
	mThermo->registerListener(this);
	mSetTemperature = 20;

	std::ofstream ofFile("TempSimulation.csv");
	ofFile << "Time;SetTemp;RoomTemp;HeaterState" << "\n";

	//Every 30 seconds
	for(time_t time = 0; time < 16 * 60 * 60; time += 30)
	{
		if (time > 240) mThermo->setPointChanged(time, 20);
		process(time);
		std::stringstream ss;

		ss << time << ";" << mSetTemperature << ";" << mRoomTemperature << ";" << mHeaterState << "\n";
		std::string outputLine = ss.str();
		std::replace(outputLine.begin(), outputLine.end(), '.', ',');
		ofFile << outputLine;

	}
	mThermo->unRegisterListener();
	delete mThermo;

	ofFile.close();
}


