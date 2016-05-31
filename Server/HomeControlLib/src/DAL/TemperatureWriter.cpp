/*
 * TemperatureWriter.cpp
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */

#include <DAL/TemperatureWriter.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <glog/logging.h>

namespace DalNs {

TemperatureWriter::TemperatureWriter()
{
}

TemperatureWriter::~TemperatureWriter()
{
}

void TemperatureWriter::sensorStarted(const std::string& sensorId)
{

}

void TemperatureWriter::sensorTemperature(const std::string& sensorId, float temperature)
{
	try
	{
		std::stringstream insert;
		insert << "INSERT INTO TempTemperatures (date, temp)";
		insert << " VALUES (NOW(), '" << temperature << "'); ";
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "hc", "bugs bunny");
		/* Connect to the MySQL test database */
		con->setSchema("HC_DB");

		stmt = con->createStatement();
		stmt->execute(insert.str());
		delete stmt;

		con->close();
		delete con;
	}
	catch (sql::SQLException &ex)
	{
		LOG(ERROR) << "clientConnected, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	}
}

void TemperatureWriter::sensorSetTemperatureUp(const std::string& sensorId)
{

}

void TemperatureWriter::sensorSetTemperatureDown(const std::string& sensorId)
{

}
} /* namespace DalNs */