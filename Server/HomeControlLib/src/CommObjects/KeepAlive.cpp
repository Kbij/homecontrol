/*
 * KeepAlive.cpp
 *
 *  Created on: Apr 23, 2016
 *      Author: koen
 */

#include <CommObjects/KeepAlive.h>
#include <sstream>

namespace CommNs {

KeepAlive::KeepAlive()
{
}

KeepAlive::~KeepAlive()
{
}

uint8_t KeepAlive::objectId() const
{
	return 0;
}

std::string KeepAlive::toString() const
{
	std::stringstream ss;
	ss << "KeepAlive, Time: " <<  std::asctime(std::localtime(&mTimeStamp));
	return ss.str();
}
} /* namespace CommNs */
