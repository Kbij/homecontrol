/*
 * HomeControlDal.cpp
 *
 *  Created on: Jun 10, 2016
 *      Author: koen
 */

#include <DAL/HomeControlDal.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <sstream>
#include <iomanip>
#include <glog/logging.h>

namespace DalNs {

HomeControlDal::HomeControlDal()
{
	//sql::Driver::threadInit();
}

HomeControlDal::~HomeControlDal()
{
}

RoomConfig* HomeControlDal::findRoomByRoomId(const std::string& roomId)
{
	LOG(INFO) << "Find room by RoomId: " << roomId;
	RoomConfig* result = nullptr;
	try
	{
		std::stringstream select;
		select << "SELECT TemperatureSensor.sensorAddress, Room.Name, Room.RoomId FROM Room ";
		select << "	INNER JOIN TemperatureSensor on Room.idRoom = TemperatureSensor.idRoom ";
		select << " WHERE Room.RoomId = '" << roomId << "'";

		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;

		/* Create a connection */
		driver = get_driver_instance();
		driver->threadInit();
		con = driver->connect("tcp://127.0.0.1:3306", "hc", "bugs bunny");
		/* Connect to the MySQL test database */
		con->setSchema("HC_DB");

		stmt = con->createStatement();
		sql::ResultSet *res =  stmt->executeQuery(select.str());

		if (res->rowsCount() > 0)
		{
			result = new RoomConfig;
			while (res->next())
			{
				result->RoomName = res->getString("Name");
				result->RoomId = res->getString("RoomId");
				result->mSensorIds.push_back(res->getString("sensorAddress"));
			}
		}
		delete res;
		delete stmt;

		con->close();
		delete con;
		driver->threadEnd();

	}
	catch (sql::SQLException &ex)
	{
		LOG(ERROR) << "clientConnected, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	}
	return result;
}

RoomConfig* HomeControlDal::findRoomBySensorId(const std::string& sensorId)
{
	LOG(INFO) << "Find room by SendorId: " << sensorId;
	RoomConfig* result = nullptr;
	try
	{
		std::stringstream select;
		select << "SELECT slaveSensor.sensorAddress, Room.Name, Room.RoomId FROM HC_DB.TemperatureSensor as masterSensor ";
		select << "	INNER JOIN Room on masterSensor.idRoom = Room.idRoom ";
		select << " INNER JOIN TemperatureSensor as slaveSensor on masterSensor.idRoom = slaveSensor.idRoom ";
		select << " WHERE masterSensor.sensorAddress = '" << sensorId << "'";

		sql::Driver *driver;

		sql::Connection *con;
		sql::Statement *stmt;

		/* Create a connection */
		driver = get_driver_instance();
		driver->threadInit();
		con = driver->connect("tcp://127.0.0.1:3306", "hc", "bugs bunny");
		/* Connect to the MySQL test database */
		con->setSchema("HC_DB");

		stmt = con->createStatement();
		sql::ResultSet *res =  stmt->executeQuery(select.str());

		if (res->rowsCount() > 0)
		{
			result = new RoomConfig;
			while (res->next())
			{
				result->RoomName = res->getString("Name");
				result->RoomId = res->getString("RoomId");
				result->mSensorIds.push_back(res->getString("sensorAddress"));
			}
		}
		delete res;
		delete stmt;

		con->close();
		delete con;
		driver->threadEnd();

	}
	catch (sql::SQLException &ex)
	{
		LOG(ERROR) << "clientConnected, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	}
	return result;
}

} /* namespace DalNs */
