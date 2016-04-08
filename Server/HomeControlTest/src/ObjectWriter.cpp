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


TEST(ObjectWriter, Constructor)
{
	DalNs::ObjectWriter* writer = new DalNs::ObjectWriter;
	delete writer;
}

TEST(ObjectWriter, WriteObject)
{
	DalNs::ObjectWriter* writer = new DalNs::ObjectWriter;
	CommNs::GpsLocation* location = new CommNs::GpsLocation(R"({"Accuracy":22,"Latitude":51.0535982,"Longitude":3.6440348,"TimeStamp":"\/Date(1459800687043+0200)\/"})");
	writer->receiveObject("test", location);
	delete location;
	delete writer;
}
