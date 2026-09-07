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
		// IFNULL: a freshly-inserted Client row (see ObjectWriter::clientConnected) has no
		// locationInterval yet, so the column is NULL until someone configures one for this
		// client - coalesce that to 0 here instead of letting the NULL->int conversion below
		// throw on every maintenance tick for a brand new client.
		select << "SELECT IFNULL(locationInterval, 0) FROM HC_DB.Client ";
		select << " WHERE clientName = '" << clientId << "'";

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);


		auto locationInterval = sess.sql(select.str()).execute();

		mysqlx::Row row = locationInterval.fetchOne();
		if (row)
		{
			result = row[0];
		}
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "locationInterval, SQLException: " << ex.what();
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

int HomeControlDal::findDevice(const std::string& deviceName)
{
	VLOG(1) << "Find device:: " << deviceName;
	int result = -1;
	try
	{
		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);

		std::stringstream insert;
		insert << "INSERT IGNORE INTO HC_DB.Client (clientName, locationInterval)";
		insert << " VALUES ('" << deviceName << "', 0); ";

		sess.sql(insert.str()).execute();

	    std::stringstream select;
		select << "SELECT idClient  FROM HC_DB.Client ";
		select << " WHERE clientName = '" << deviceName << "'";

		auto deviceSelect = sess.sql(select.str()).execute();

		mysqlx::Row row = deviceSelect.fetchOne();
		result = row[0];
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "clientConnected, SQLException: " << ex.what();
	}

	VLOG(1) << "DeviceId for client: " << deviceName << ": " << result;
	return result;
}

void HomeControlDal::logLocation(int deviceId, double lat, double lon, double accuracy, double batteryLevel, time_t timestamp)
{
	VLOG(1) << "Log location: " << deviceId;
	try
	{
		std::stringstream insertCmd;
		std::tm *tm = std::localtime(&timestamp);
		char buffer[32];
		std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S",tm);
		insertCmd << "INSERT INTO Location (idClient,  latitude, longitude, accuracy, timestamp) ";
		insertCmd << " Values (" << deviceId << ", " << lat << ", " << lon << ", " << accuracy << ", '" << buffer << "')";
		VLOG(1) << "insertCmd: " << insertCmd.str();

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);

		sess.sql(insertCmd.str()).execute();

		std::stringstream update;
		update << "UPDATE Client SET lastMessage = NOW(), batteryLevel = " << batteryLevel << " Where idClient = " << deviceId;
		VLOG(1) << "update: " << update.str();
		sess.sql(update.str()).execute();
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "logLocation, SQLException: " << ex.what();
	}
}

std::string HomeControlDal::adminCode(const std::string& clientId)
{
	VLOG(1) << "Find admin code for client: " << clientId;
	std::string result;
	try
	{
		std::stringstream select;
		// IFNULL: a client with no admin PIN configured has a NULL adminCode column;
		// coalesce that to "" so the string<->NULL conversion doesn't throw, same as
		// locationInterval() does above for its own nullable column.
		select << "SELECT IFNULL(adminCode, '') FROM HC_DB.Client ";
		select << " WHERE clientName = '" << clientId << "'";

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);

		auto adminCodeResult = sess.sql(select.str()).execute();

		mysqlx::Row row = adminCodeResult.fetchOne();
		if (row)
		{
			result = (std::string) row[0];
		}
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "adminCode, SQLException: " << ex.what();
	}

	return result;
}

std::vector<std::string> HomeControlDal::allClientNames()
{
	VLOG(1) << "Find all client names";
	std::vector<std::string> result;
	try
	{
		std::stringstream select;
		select << "SELECT clientName FROM HC_DB.Client ORDER BY clientName";

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);

		auto clientResult = sess.sql(select.str()).execute();
		for (mysqlx::Row row : clientResult.fetchAll())
		{
			result.push_back((std::string) row[0]);
		}
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "allClientNames, SQLException: " << ex.what();
	}

	return result;
}

std::vector<LocationPoint> HomeControlDal::locationHistory(const std::string& clientId, int minutes)
{
	VLOG(1) << "Location history for client: " << clientId << ", minutes: " << minutes;
	std::vector<LocationPoint> result;
	try
	{
		std::stringstream select;
		select << "SELECT latitude, longitude, UNIX_TIMESTAMP(timestamp) FROM Location ";
		select << " INNER JOIN Client ON Location.idClient = Client.idClient ";
		select << " WHERE Client.clientName = '" << clientId << "' ";
		select << " AND timestamp >= NOW() - INTERVAL " << minutes << " MINUTE ";
		select << " ORDER BY timestamp ASC";

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);

		auto historyResult = sess.sql(select.str()).execute();
		for (mysqlx::Row row : historyResult.fetchAll())
		{
			LocationPoint point;
			point.Latitude = row[0];
			point.Longitude = row[1];
			point.Timestamp = (time_t)(int64_t) row[2];
			result.push_back(point);
		}
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "locationHistory, SQLException: " << ex.what();
	}

	VLOG(1) << "Location history for client: " << clientId << ": " << result.size() << " points";
	return result;
}

void HomeControlDal::updateGeofence(const std::string& clientId, double lat, double lon, double radiusMeters)
{
	VLOG(1) << "Update geofence for client: " << clientId << ", lat: " << lat << ", lon: " << lon << ", radius: " << radiusMeters;
	try
	{
		std::stringstream update;
		// geofenceCreatedAt is only set the *first* time this fires for a fresh geofence
		// (i.e. while it's still NULL) - every later call (a 30-min renewal, or a repeated
		// report of the same fence) only touches geofenceUpdatedAt, leaving the original
		// creation time and center/radius alone. That's what makes CreatedAt vs. UpdatedAt
		// meaningful to show separately on the admin map (see AdminController).
		update << "UPDATE HC_DB.Client SET ";
		update << " geofenceLat = " << lat << ", geofenceLon = " << lon << ", geofenceRadius = " << radiusMeters << ", ";
		update << " geofenceCreatedAt = IF(geofenceCreatedAt IS NULL, NOW(), geofenceCreatedAt), ";
		update << " geofenceUpdatedAt = NOW() ";
		update << " WHERE clientName = '" << clientId << "'";

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);
		sess.sql(update.str()).execute();
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "updateGeofence, SQLException: " << ex.what();
	}
}

void HomeControlDal::clearGeofence(const std::string& clientId)
{
	VLOG(1) << "Clear geofence for client: " << clientId;
	try
	{
		std::stringstream update;
		update << "UPDATE HC_DB.Client SET ";
		update << " geofenceLat = NULL, geofenceLon = NULL, geofenceRadius = NULL, ";
		update << " geofenceCreatedAt = NULL, geofenceUpdatedAt = NULL ";
		update << " WHERE clientName = '" << clientId << "'";

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);
		sess.sql(update.str()).execute();
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "clearGeofence, SQLException: " << ex.what();
	}
}

time_t HomeControlDal::lastMessage(const std::string& clientId)
{
	VLOG(1) << "Find last message time for client: " << clientId;
	time_t result = 0;
	try
	{
		std::stringstream select;
		// IFNULL, same reasoning as locationInterval()/adminCode() above.
		select << "SELECT IFNULL(UNIX_TIMESTAMP(lastMessage), 0) FROM HC_DB.Client ";
		select << " WHERE clientName = '" << clientId << "'";

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);

		auto lastMessageResult = sess.sql(select.str()).execute();

		mysqlx::Row row = lastMessageResult.fetchOne();
		if (row)
		{
			result = (time_t)(int64_t) row[0];
		}
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "lastMessage, SQLException: " << ex.what();
	}

	return result;
}

GeofenceInfo HomeControlDal::geofence(const std::string& clientId)
{
	GeofenceInfo result{false, 0.0, 0.0, 0.0, 0, 0};
	try
	{
		std::stringstream select;
		// IFNULL throughout, same reasoning as locationInterval()/adminCode() above; the
		// explicit "geofenceCreatedAt IS NOT NULL" column is the one field that actually
		// distinguishes "no active geofence" from "one exists with (unlikely) zero values".
		select << "SELECT IFNULL(geofenceLat, 0), IFNULL(geofenceLon, 0), IFNULL(geofenceRadius, 0), ";
		select << " IFNULL(UNIX_TIMESTAMP(geofenceCreatedAt), 0), IFNULL(UNIX_TIMESTAMP(geofenceUpdatedAt), 0), ";
		select << " (geofenceCreatedAt IS NOT NULL) ";
		select << " FROM HC_DB.Client WHERE clientName = '" << clientId << "'";

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);
		auto geofenceResult = sess.sql(select.str()).execute();

		mysqlx::Row row = geofenceResult.fetchOne();
		if (row)
		{
			result.Latitude = row[0];
			result.Longitude = row[1];
			result.RadiusMeters = row[2];
			result.CreatedAt = (time_t)(int64_t) row[3];
			result.UpdatedAt = (time_t)(int64_t) row[4];
			result.Active = ((int) row[5]) != 0;
		}
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "geofence, SQLException: " << ex.what();
	}

	return result;
}

} /* namespace DalNs */
