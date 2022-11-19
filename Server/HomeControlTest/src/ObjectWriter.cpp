/*
 * ObjectWriter.cpp
 *
 *  Created on: Apr 8, 2016
 *      Author: koen
 */


#include "DAL/ObjectWriter.h"
#include "CommObjects/GpsLocation.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>

namespace
{
const std::string SERVER = "tcp://192.168.10.7:3306";
const std::string USER = "hc";
const std::string PWD = "bugs bunny";
const int PORT = 3306;
const std::string DB = "hc";
}

TEST(ObjectWriter, Constructor)
{
	DalNs::ObjectWriter* writer = new DalNs::ObjectWriter(SERVER, PORT, DB, USER, PWD);
	delete writer;
}

TEST(ObjectWriter, WriteObject)
{
	DalNs::ObjectWriter* writer = new DalNs::ObjectWriter(SERVER, PORT, DB, USER, PWD);
	CommNs::GpsLocation* location = new CommNs::GpsLocation(R"({"Accuracy":22,"Latitude":51.0535982,"Longitude":3.6440348,"TimeStamp":"\/Date(1459800687043+0200)\/"})");
	writer->receiveObject("test", location);
	delete location;
	delete writer;
}
