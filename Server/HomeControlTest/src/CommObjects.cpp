/*
 * CommObjects.cpp
 *
 *  Created on: Apr 6, 2016
 *      Author: koen
 */

#include "CommObjects/GpsLocation.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <iostream>

TEST(CommObjects, EmptyGpsConstructor)
{
	CommNs::GpsLocation* location = new CommNs::GpsLocation("");
	EXPECT_EQ(0, location->accuracy());
	EXPECT_EQ(0, location->latitude());
	EXPECT_EQ(0, location->longitude());
	EXPECT_EQ(0, location->timeStamp());
	delete location;
}

TEST(CommObjects, JsonGpsConstructor)
{
	CommNs::GpsLocation* location = new CommNs::GpsLocation(R"({"Accuracy":22,"Latitude":51.0535982,"Longitude":3.6440348,"TimeStamp":"\/Date(1459800687043+0200)\/"})");
	EXPECT_EQ(22, location->accuracy());
	EXPECT_EQ(51.0535982, location->latitude());
	EXPECT_EQ(3.6440348, location->longitude());
	EXPECT_EQ(1459800687, location->timeStamp());
	delete location;
}
