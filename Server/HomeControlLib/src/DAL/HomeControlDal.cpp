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

HomeControlDal::HomeControlDal(const std::string& server, const std::string& user, const std::string& pwd):
	mServer(server),
	mUser(user),
	mPwd(pwd)
{
	LOG(INFO) << "SQL Server: " << server;
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
		select << "SELECT TemperatureSensor.sensorAddress, Room.Name, Room.RoomId, Room.HeaterOutput FROM Room ";
		select << "	INNER JOIN TemperatureSensor on Room.idRoom = TemperatureSensor.idRoom ";
		select << " WHERE Room.RoomId = '" << roomId << "'";

		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;

		/* Create a connection */
		driver = get_driver_instance();
		driver->threadInit();
		con = driver->connect(mServer, mUser, mPwd);
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
				result->HeaterOutput = res->getInt("HeaterOutput");
				result->SensorIds.push_back(res->getString("sensorAddress"));
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

	if (result == nullptr)
	{
		LOG(INFO) << "Room not found";
	}
	else
	{
		LOG(INFO) << "Room found: " << result->RoomName << ", config: " <<  result->toString();
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
		select << "SELECT slaveSensor.sensorAddress, Room.Name, Room.RoomId, Room.HeaterOutput FROM HC_DB.TemperatureSensor as masterSensor ";
		select << "	INNER JOIN Room on masterSensor.idRoom = Room.idRoom ";
		select << " INNER JOIN TemperatureSensor as slaveSensor on masterSensor.idRoom = slaveSensor.idRoom ";
		select << " WHERE masterSensor.sensorAddress = '" << sensorId << "'";

		sql::Driver *driver;

		sql::Connection *con;
		sql::Statement *stmt;

		/* Create a connection */
		driver = get_driver_instance();
		driver->threadInit();
		con = driver->connect(mServer, mUser, mPwd);
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
				result->HeaterOutput = res->getInt("HeaterOutput");
				result->SensorIds.push_back(res->getString("sensorAddress"));
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
	if (result == nullptr)
	{
		LOG(INFO) << "Room not found";
	}
	else
	{
		LOG(INFO) << "Room found: " << result->RoomName << ", config: " <<  result->toString();
	}
	return result;
}

double HomeControlDal::getSensorCalibration(const std::string& sensorId)
{
	LOG(INFO) << "Find Calibration for SendorId: " << sensorId;
	double result = 0;
	try
	{
		std::stringstream select;
		select << "SELECT calibration FROM HC_DB.TemperatureSensor  ";
		select << " WHERE sensorAddress = '" << sensorId << "'";

		sql::Driver *driver;

		sql::Connection *con;
		sql::Statement *stmt;

		/* Create a connection */
		driver = get_driver_instance();
		driver->threadInit();
		con = driver->connect(mServer, mUser, mPwd);
		/* Connect to the MySQL test database */
		con->setSchema("HC_DB");

		stmt = con->createStatement();
		sql::ResultSet *res =  stmt->executeQuery(select.str());

		if (res->rowsCount() > 0)
		{
			while (res->next())
			{
				result = res->getDouble("calibration");
			}
		}
		else
		{
			LOG(ERROR) << "Calibration not found";
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

	LOG(INFO) << "Calibration for sensor: " << sensorId << ": " << result;
	return result;
}

int HomeControlDal::locationInterval(const std::string& clientId)
{
	VLOG(1) << "Find location interval for client: " << clientId;
	int result = 0;
	try
	{
		std::stringstream select;
		select << "SELECT locationInterval  FROM HC_DB.Client ";
		select << " WHERE clientName = '" << clientId << "'";

		sql::Driver *driver;

		sql::Connection *con;
		sql::Statement *stmt;

		/* Create a connection */
		driver = get_driver_instance();
		driver->threadInit();
		con = driver->connect(mServer, mUser, mPwd);
		/* Connect to the MySQL test database */
		con->setSchema("HC_DB");

		stmt = con->createStatement();
		sql::ResultSet *res =  stmt->executeQuery(select.str());

		if (res->rowsCount() > 0)
		{
			while (res->next())
			{
				result = res->getInt("locationInterval");
			}
		}
		else
		{
			LOG(ERROR) << "Interval not found for client: " << clientId;
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

	VLOG(1) << "Location interval for client: " << clientId << ": " << result;
	return result;
}

} /* namespace DalNs */
