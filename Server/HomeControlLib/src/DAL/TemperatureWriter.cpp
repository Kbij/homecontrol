/*
 * TemperatureWriter.cpp
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */

#include <DAL/TemperatureWriter.h>
// #include <cppconn/driver.h>
// #include <cppconn/exception.h>
// #include <cppconn/resultset.h>
// #include <cppconn/statement.h>
#include <Comm/TemperatureSourceIf.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <glog/logging.h>

namespace DalNs {

TemperatureWriter::TemperatureWriter(CommNs::TemperatureSourceIf* sensors):
	mSensors(sensors)
{
	if (mSensors)
	{
		mSensors->registerTemperatureListener(this);
	}
}

TemperatureWriter::~TemperatureWriter()
{
	if (mSensors)
	{
		mSensors->unRegisterTemperatureListener(this);
	}
}

void TemperatureWriter::sensorStarted(const std::string& sensorId)
{
	// try
	// {
	// 	VLOG(1) << "Received sensor start for: " << sensorId;
	// 	std::stringstream insert;
	// 	insert << "INSERT IGNORE INTO TemperatureSensor (sensorAddress)";
	// 	insert << " VALUES ('" << sensorId << "'); ";

	// 	sql::Driver *driver;
	// 	sql::Connection *con;
	// 	sql::Statement *stmt;

	// 	/* Create a connection */
	// 	driver = get_driver_instance();
	// 	driver->threadInit();

	// 	con = driver->connect("tcp://127.0.0.1:3306", "hc", "bugs bunny");

	// 	con->setSchema("HC_DB");

	// 	stmt = con->createStatement();
	// 	stmt->execute(insert.str());
	// 	delete stmt;

	// 	con->close();
	// 	delete con;
	// 	driver->threadEnd();
	// }
	// catch (sql::SQLException &ex)
	// {
	// 	LOG(ERROR) << "clientConnected, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	// }
}

void TemperatureWriter::sensorTemperature(const std::string& sensorId, double temperature)
{
	// try
	// {
	// 	VLOG(1) << "Received temperature: " << temperature << ", from sensor: " << sensorId;

	// 	std::stringstream insert;
	// 	insert << "INSERT INTO Temperature (idSensor, temperature, date) ";
	// 	insert << " SELECT TemperatureSensor.idTemperatureSensor, " << temperature << ", NOW() ";
	// 	insert << " FROM TemperatureSensor where sensorAddress = '" << sensorId << "'";

	// 	sql::Driver *driver;
	// 	sql::Connection *con;
	// 	sql::Statement *stmt;

	// 	/* Create a connection */
	// 	driver = get_driver_instance();
	// 	driver->threadInit();
	// 	con = driver->connect("tcp://127.0.0.1:3306", "hc", "bugs bunny");

	// 	con->setSchema("HC_DB");

	// 	stmt = con->createStatement();
	// 	stmt->execute(insert.str());
	// 	delete stmt;

	// 	con->close();
	// 	delete con;
	// 	driver->threadEnd();
	// }
	// catch (sql::SQLException &ex)
	// {
	// 	LOG(ERROR) << "clientConnected, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	// }
}

void TemperatureWriter::sensorSetTemperatureUp(const std::string& sensorId)
{

}

void TemperatureWriter::sensorSetTemperatureDown(const std::string& sensorId)
{

}
} /* namespace DalNs */
