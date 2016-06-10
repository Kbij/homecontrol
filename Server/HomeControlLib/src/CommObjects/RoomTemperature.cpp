/*
 * RoomTemperature.cpp
 *
 *  Created on: Jun 8, 2016
 *      Author: koen
 */

#include <CommObjects/RoomTemperature.h>
#include <sstream>

namespace CommNs {

RoomTemperature::RoomTemperature(double temperature):
	mRoomTemperature(temperature)
{

}
RoomTemperature::RoomTemperature(const std::string& json):
	mRoomTemperature(0)
{
}

RoomTemperature::~RoomTemperature()
{
}

uint8_t RoomTemperature::objectId() const
{
	return 20;
}

std::string RoomTemperature::toString() const
{
	std::stringstream ss;
	ss << "Roomtemperature, Temperature: " <<  mRoomTemperature;
	return ss.str();
}

std::string RoomTemperature::json() const
{
	return "";
}
} /* namespace CommNs */
