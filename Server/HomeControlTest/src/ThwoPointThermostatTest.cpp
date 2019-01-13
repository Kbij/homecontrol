/*
 * ThwoPointThermostatTest.cpp
 *
 *  Created on: Jan 13, 2019
 *      Author: koen
 */

#include "Logic/TwoPointThermostat.h"
#include "Logic/ThermostatListenerIf.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"

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

TEST(TwoPointThermostat, Connstructor)
{
	ThermoListenerStub* listener = new ThermoListenerStub;
	LogicNs::TwoPointThermostat* thermo = new LogicNs::TwoPointThermostat(0.5);
	thermo->registerListener(listener);
	thermo->unRegisterListener();
	delete thermo;
}

TEST(TwoPointThermostat, HeaterOn)
{
	ThermoListenerStub* listener = new ThermoListenerStub;
	LogicNs::TwoPointThermostat* thermo = new LogicNs::TwoPointThermostat(0.5);
	thermo->registerListener(listener);
	thermo->setPointChanged(0, 21);

	//Way to cold; must be switched on
	thermo->temperatureChanged(0, 16);
	EXPECT_TRUE(listener->mHeaterOnCalled);

	thermo->unRegisterListener();
	delete thermo;
}

TEST(TwoPointThermostat, HeaterOff)
{
	ThermoListenerStub* listener = new ThermoListenerStub;
	LogicNs::TwoPointThermostat* thermo = new LogicNs::TwoPointThermostat(0.5);
	thermo->registerListener(listener);
	thermo->setPointChanged(0, 21);

	//Way to hot
	thermo->temperatureChanged(0, 25);
	EXPECT_TRUE(listener->mHeaterOffCalled);

	thermo->unRegisterListener();
	delete thermo;
}

TEST(TwoPointThermostat, Hysteresis)
{
	ThermoListenerStub* listener = new ThermoListenerStub;
	LogicNs::TwoPointThermostat* thermo = new LogicNs::TwoPointThermostat(2);
	thermo->registerListener(listener);
	thermo->setPointChanged(0, 21);

	//Way to cold; must be switched on
	thermo->temperatureChanged(0, 16);
	EXPECT_TRUE(listener->mHeaterOnCalled);
	listener->mHeaterOnCalled = false;
	listener->clear();

	//New temperature, still on
	thermo->temperatureChanged(0, 16.1);
	EXPECT_TRUE(listener->mHeaterOnCalled);
	listener->clear();

	//Little bit above set
	thermo->temperatureChanged(0, 21.1);
	EXPECT_TRUE(listener->mHeaterOnCalled);
	listener->clear();

	//Above hysteresis
	thermo->temperatureChanged(0, 22.5);
	EXPECT_TRUE(listener->mHeaterOffCalled);
	listener->clear();

	//Way to hot
	thermo->temperatureChanged(0, 25);
	EXPECT_TRUE(listener->mHeaterOffCalled);
	listener->clear();

	thermo->temperatureChanged(0, 20.80);
	EXPECT_TRUE(listener->mHeaterOffCalled);
	listener->clear();

	thermo->temperatureChanged(0, 19.7);
	EXPECT_TRUE(listener->mHeaterOnCalled);
	listener->clear();

	thermo->temperatureChanged(0, 21);
	EXPECT_TRUE(listener->mHeaterOnCalled);
	listener->clear();

	thermo->temperatureChanged(0, 21);
	EXPECT_TRUE(listener->mHeaterOnCalled);
	listener->clear();

	thermo->temperatureChanged(0, 25);
	EXPECT_TRUE(listener->mHeaterOffCalled);
	listener->clear();

	thermo->unRegisterListener();
	delete thermo;
}
