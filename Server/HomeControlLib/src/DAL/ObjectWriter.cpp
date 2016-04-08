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
	// TODO Auto-generated constructor stub

}

ObjectWriter::~ObjectWriter()
{
	// TODO Auto-generated destructor stub
}
void ObjectWriter::receiveObject(const std::string name, const CommNs::CommObjectIf* object)
{
	if (object == nullptr) return;
	try
	{
		std::string insertCmd;
		if (object->objectId() == 10)
		{
			if(const CommNs::GpsLocation* location = dynamic_cast<const CommNs::GpsLocation*> (object))
			{
				std::stringstream ss;
				time_t time = location->timeStamp();
				std::tm *tm = std::localtime(&time);
				char buffer[32];
				std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S",tm);
//				std::cout << "Time: " << buffer << std::endl;

				ss << "INSERT INTO Locations (client, latitude, longitude, accuracy, timestamp) VALUES "
					" ('" << name << "', " << location->latitude() << ", " << location->longitude() << ", " << location->accuracy() << ", '" << buffer << "');";
				insertCmd = ss.str();

			}
		}

		if (object->objectId() == 11)
		{
			if(const CommNs::MessageObject* message = dynamic_cast<const CommNs::MessageObject*> (object))
			{
				std::stringstream ss;
				time_t time = message->timeStamp();
				std::tm *tm = std::localtime(&time);
				char buffer[32];
				std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S",tm);

				ss << "INSERT INTO Locations (client, message, timestamp) VALUES "
					" ('" << name << "', '" << message->message() << "' , '" << buffer << "');";
				insertCmd = ss.str();
			}
		}

		if (insertCmd != "")
		{
			sql::Driver *driver;
			sql::Connection *con;
			sql::Statement *stmt;

			/* Create a connection */
			driver = get_driver_instance();
			con = driver->connect("tcp://127.0.0.1:3306", "root", "bugs bunny");
			/* Connect to the MySQL test database */
			con->setSchema("HC_DB");

			stmt = con->createStatement();
			stmt->execute(insertCmd);
			delete stmt;
			delete con;
		}
	}
	catch (sql::SQLException &ex)
	{
		LOG(ERROR) << "SQLExceptin: " << ex.what() << ", MySQL error code: " << ex.getErrorCode() << ", SQLState: " << ex.getSQLState();
	}

}
} /* namespace DalNs */
