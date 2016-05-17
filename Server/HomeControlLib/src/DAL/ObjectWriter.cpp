/*
 * ObjectWriter.cpp
 *
 *  Created on: Apr 8, 2016
 *      Author: koen
 */

#include "ObjectWriter.h"
#include "CommObjects/CommObjectIf.h"
#include "CommObjects/GpsLocation.h"
#include "CommObjects/MessageObject.h"
#include "CommObjects/KeepAlive.h"
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

ObjectWriter::ObjectWriter()
{
}

ObjectWriter::~ObjectWriter()
{
}

void ObjectWriter::clientConnected(const std::string& name)
{
	LOG(INFO) << "Client connected: " << name;
	try
	{
		std::stringstream insert;
		insert << "INSERT IGNORE INTO Client (clientName)";
		insert << " VALUES ('" << name << "'); ";
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

		std::stringstream update;
		update << "UPDATE Client SET lastMessage = NOW() WHERE clientName = '" << name << "';";

		stmt = con->createStatement();
		stmt->execute(update.str());
		delete stmt;

		con->close();
		delete con;
	}
	catch (sql::SQLException &ex)
	{
		LOG(ERROR) << "clientConnected, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	}
}

void ObjectWriter::clientDisConnected(const std::string& name)
{
	LOG(INFO) << "Client disconnected: " << name;
}

void ObjectWriter::receiveObject(const std::string name, const CommNs::CommObjectIf* object)
{
	if (object == nullptr) return;
	try
	{
		std::string insertCmd;

		if (object->objectId() == 0)
		{
			if(const CommNs::KeepAlive* keepAlive = dynamic_cast<const CommNs::KeepAlive*> (object))
			{
				LOG(INFO) << "Keep alive received";
			}
		}

		if (object->objectId() == 10)
		{
			if(const CommNs::GpsLocation* location = dynamic_cast<const CommNs::GpsLocation*> (object))
			{
				std::stringstream ss;
				time_t time = location->timeStamp();
				std::tm *tm = std::localtime(&time);
				char buffer[32];
				std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S",tm);
				ss << "INSERT INTO Location (idClient,  latitude, longitude, accuracy, timestamp) ";
				ss << " SELECT idClient, " << location->latitude() << ", " << location->longitude() << ", " << location->accuracy() << ", '" << buffer << "' FROM Client where clientName = '" << name << "' ";
				insertCmd = ss.str();

			}
		}

		if (object->objectId() == 11)
		{
			if(const CommNs::MessageObject* message = dynamic_cast<const CommNs::MessageObject*> (object))
			{
				LOG(INFO) << "Message received";
			}
		}



		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "hc", "bugs bunny");
		/* Connect to the MySQL test database */
		con->setSchema("HC_DB");

		if  (insertCmd != "")
		{
			stmt = con->createStatement();
			stmt->execute(insertCmd);
			delete stmt;
		}
		std::stringstream update;
		update << "UPDATE Client SET lastMessage = NOW() WHERE clientName = '" << name << "';";

		stmt = con->createStatement();
		stmt->execute(update.str());
		delete stmt;


		con->close();
		delete con;
	}
	catch (sql::SQLException &ex)
	{
		LOG(ERROR) << "receiveObject, SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	}

}
} /* namespace DalNs */
