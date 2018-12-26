/*
 * LocationInterval.cpp
 *
 *  Created on: Dec 25, 2018
 *      Author: koen
 */

#include <CommObjects/LocationInterval.h>
#include <json/json.h>

namespace CommNs {

LocationInterval::LocationInterval(int interval):
	mInterval(interval)
{
}

LocationInterval::~LocationInterval()
{

}

uint8_t LocationInterval::objectId() const
{
	return 30;
}

std::string LocationInterval::toString() const
{
	return "Location Interval";
}

std::string LocationInterval::json() const
{
	Json::Value root;
	root["interval"] = mInterval;

	Json::FastWriter writer;
	return writer.write(root);
}
} /* namespace CommNs */
