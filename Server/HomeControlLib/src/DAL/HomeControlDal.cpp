/*
 * HomeControlDal.cpp
 *
 *  Created on: Jun 10, 2016
 *      Author: koen
 */

#include <DAL/HomeControlDal.h>
#include <mysqlx/xdevapi.h>
#include <sstream>
#include <iomanip>
#include <glog/logging.h>


namespace DalNs {

HomeControlDal::HomeControlDal(const std::string& server, int port, const std::string& db, const std::string& user, const std::string& pwd):
	mServer(server),
	mPort(port),
	mDb(db),
	mUser(user),
	mPwd(pwd),
	mHeaterState()
{
	LOG(INFO) << "SQL Server: " << server;
}

HomeControlDal::~HomeControlDal()
{
}

RoomConfig* HomeControlDal::findRoomByRoomId(const std::string& roomId)
{
	LOG(INFO) << "Find room by RoomId: " << roomId;
	RoomConfig* result = nullptr;
	// try
	// {
	// 	std::stringstream select;
	// 	select << "SELECT TemperatureSensor.sensorAddress, Room.Name, Room.RoomId, Room.HeaterOutput FROM Room ";
	// 	select << "	INNER JOIN TemperatureSensor on Room.idRoom = TemperatureSensor.idRoom ";
	// 	select << " WHERE Room.RoomId = '" << roomId << "'";

	// 	sql::Driver *driver;
	// 	sql::Connection *con;
	// 	sql::Statement *stmt;

	// 	/* Create a connection */
	// 	driver = get_driver_instance();
	// 	driver->threadInit();
	// 	con = driver->connect(mServer, mUser, mPwd);
	// 	/* Connect to the MySQL test database */
	// 	con->setSchema("HC_DB");

	// 	stmt = con->createStatement();
	// 	sql::ResultSet *res =  stmt->executeQuery(select.str());

	// 	if (res->rowsCount() > 0)
	// 	{
	// 		result = new RoomConfig;
	// 		while (res->next())
	// 		{
	// 			result->RoomName = res->getString("Name");
	// 			result->RoomId = res->getString("RoomId");
	// 			result->HeaterOutput = res->getInt("HeaterOutput");
	// 			result->SensorIds.push_back(res->getString("sensorAddress"));
	// 		}
	// 	}
	// 	delete res;
	// 	delete stmt;

	// 	con->close();
	// 	delete con;
	// 	driver->threadEnd();

	// }
	// catch (sql::SQLException &ex)
	// {
	// 	LOG(ERROR) << "clientConnected, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	// }

	// if (result == nullptr)
	// {
	// 	LOG(INFO) << "Room not found";
	// }
	// else
	// {
	// 	LOG(INFO) << "Room found: " << result->RoomName << ", config: " <<  result->toString();
	// }
	return result;
}

RoomConfig* HomeControlDal::findRoomBySensorId(const std::string& sensorId)
{
	LOG(INFO) << "Find room by SendorId: " << sensorId;
	RoomConfig* result = nullptr;
	// try
	// {
	// 	std::stringstream select;
	// 	select << "SELECT slaveSensor.sensorAddress, Room.Name, Room.RoomId, Room.HeaterOutput FROM HC_DB.TemperatureSensor as masterSensor ";
	// 	select << "	INNER JOIN Room on masterSensor.idRoom = Room.idRoom ";
	// 	select << " INNER JOIN TemperatureSensor as slaveSensor on masterSensor.idRoom = slaveSensor.idRoom ";
	// 	select << " WHERE masterSensor.sensorAddress = '" << sensorId << "'";

	// 	sql::Driver *driver;

	// 	sql::Connection *con;
	// 	sql::Statement *stmt;

	// 	/* Create a connection */
	// 	driver = get_driver_instance();
	// 	driver->threadInit();
	// 	con = driver->connect(mServer, mUser, mPwd);
	// 	/* Connect to the MySQL test database */
	// 	con->setSchema("HC_DB");

	// 	stmt = con->createStatement();
	// 	sql::ResultSet *res =  stmt->executeQuery(select.str());

	// 	if (res->rowsCount() > 0)
	// 	{
	// 		result = new RoomConfig;
	// 		while (res->next())
	// 		{
	// 			result->RoomName = res->getString("Name");
	// 			result->RoomId = res->getString("RoomId");
	// 			result->HeaterOutput = res->getInt("HeaterOutput");
	// 			result->SensorIds.push_back(res->getString("sensorAddress"));
	// 		}
	// 	}
	// 	delete res;
	// 	delete stmt;

	// 	con->close();
	// 	delete con;
	// 	driver->threadEnd();

	// }
	// catch (sql::SQLException &ex)
	// {
	// 	LOG(ERROR) << "clientConnected, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	// }
	// if (result == nullptr)
	// {
	// 	LOG(INFO) << "Room not found";
	// }
	// else
	// {
	// 	LOG(INFO) << "Room found: " << result->RoomName << ", config: " <<  result->toString();
	// }
	return result;
}

double HomeControlDal::getSensorCalibration(const std::string& sensorId)
{
	LOG(INFO) << "Find Calibration for SendorId: " << sensorId;
	double result = 0;
	// try
	// {
	// 	std::stringstream select;
	// 	select << "SELECT calibration FROM HC_DB.TemperatureSensor  ";
	// 	select << " WHERE sensorAddress = '" << sensorId << "'";

	// 	sql::Driver *driver;

	// 	sql::Connection *con;
	// 	sql::Statement *stmt;

	// 	/* Create a connection */
	// 	driver = get_driver_instance();
	// 	driver->threadInit();
	// 	con = driver->connect(mServer, mUser, mPwd);
	// 	/* Connect to the MySQL test database */
	// 	con->setSchema("HC_DB");

	// 	stmt = con->createStatement();
	// 	sql::ResultSet *res =  stmt->executeQuery(select.str());

	// 	if (res->rowsCount() > 0)
	// 	{
	// 		while (res->next())
	// 		{
	// 			result = res->getDouble("calibration");
	// 		}
	// 	}
	// 	else
	// 	{
	// 		LOG(ERROR) << "Calibration not found";
	// 	}
	// 	delete res;
	// 	delete stmt;

	// 	con->close();
	// 	delete con;
	// 	driver->threadEnd();

	// }
	// catch (sql::SQLException &ex)
	// {
	// 	LOG(ERROR) << "clientConnected, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	// }

	// LOG(INFO) << "Calibration for sensor: " << sensorId << ": " << result;
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

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);


		auto locationInterval = sess.sql(select.str()).execute();

		mysqlx::Row row = locationInterval.fetchOne();
		result = row[0];
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "clientConnected, SQLException: " << ex.what();
	}

	VLOG(1) << "Location interval for client: " << clientId << ": " << result;
	return result;
}

void HomeControlDal::writeHeaterOn(const std::string& roomId)
{
	bool writeState = false;
	//Force the first write
	if (mHeaterState.find(roomId) == mHeaterState.end())
	{
		writeState = true;
	}
	else
	{
		//State has changed
		if (!mHeaterState[roomId]) writeState = true;
	}

	mHeaterState[roomId] = true;

	if (writeState)
	{
		writeHeaterState(roomId, true);
	}
}

void HomeControlDal::writeHeaterOff(const std::string& roomId)
{
	bool writeState = false;
	//Force the first write
	if (mHeaterState.find(roomId) == mHeaterState.end())
	{
		writeState = true;
	}
	else
	{
		//State has changed
		if (mHeaterState[roomId]) writeState = true;
	}

	mHeaterState[roomId] = false;

	if (writeState)
	{
		writeHeaterState(roomId, false);
	}
}

void HomeControlDal::writeHeaterState(const std::string& roomId, bool state)
{
	// try
	// {
	// 	VLOG(1) << "Writing heaterstate: " << state  << ", from room: " << roomId;

	// 	std::stringstream insert;
	// 	insert << "INSERT INTO RoomHeaterState(idRoom, heater, date) ";
	// 	insert << " SELECT idRoom, " << state << ", NOW() FROM Room WHERE RoomId = '" << roomId << "'";

	// 	sql::Driver *driver;
	// 	sql::Connection *con;
	// 	sql::Statement *stmt;

	// 	/* Create a connection */
	// 	driver = get_driver_instance();
	// 	driver->threadInit();
	// 	con = driver->connect(mServer, mUser, mPwd);

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
	// 	LOG(ERROR) << "Write heater state, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	// }

}
} /* namespace DalNs */
