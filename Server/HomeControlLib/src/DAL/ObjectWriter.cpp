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
#include <mysqlx/xdevapi.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <glog/logging.h>

namespace DalNs {

ObjectWriter::ObjectWriter(const std::string& server, int port, const std::string& db, const std::string& user, const std::string& pwd):
	mServer(server),
	mPort(port),
	mDb(db),
	mUser(user),
	mPwd(pwd)
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
		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);

		std::stringstream insert;
		insert << "INSERT IGNORE INTO Client (clientName)";
		insert << " VALUES ('" << name << "'); ";

		sess.sql(insert.str()).execute();

		std::stringstream update;
		update << "UPDATE Client SET lastMessage = NOW() WHERE clientName = '" << name << "';";
		
		sess.sql(update.str()).execute();
	}
	catch(const std::exception& ex)
	{
		LOG(ERROR) << "Exception on ClientConnected, Exception: " << ex.what();
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
		int batteryLevel = -1;
		if (object->objectId() == 0)
		{
			if(const CommNs::KeepAlive* keepAlive = dynamic_cast<const CommNs::KeepAlive*> (object))
			{
				VLOG(1) << name << ", keep alive received";
				(void) keepAlive;
			}
		}

		if (object->objectId() == 10)
		{
			if(const CommNs::GpsLocation* location = dynamic_cast<const CommNs::GpsLocation*> (object))
			{
				batteryLevel = location->batteryLevel();
				VLOG(1) << name << ", location received: " << location->toString();
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
				VLOG(1) << name << ", message received";
				(void) message;
			}
		}

		mysqlx::Session sess(mServer, mPort, mUser, mPwd, mDb);

		if  (insertCmd != "")
		{
			sess.sql(insertCmd).execute();
		}
		std::stringstream update;
		update << "UPDATE Client SET lastMessage = NOW()";
		if (batteryLevel != -1)
		{
			update << ", batteryLevel = " << batteryLevel;
		}
		update << " WHERE clientName = '" << name << "';";

		sess.sql(update.str()).execute();

	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "receiveObject, SQLException: " << ex.what();
	}

}
} /* namespace DalNs */
