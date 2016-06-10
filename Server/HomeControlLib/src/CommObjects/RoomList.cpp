/*
 * RoomList.cpp
 *
 *  Created on: Jun 10, 2016
 *      Author: koen
 */

#include <CommObjects/RoomList.h>

namespace CommNs {

RoomList::RoomList()
{

}

RoomList::RoomList(const std::string& json)
{

}

RoomList::~RoomList()
{

}

uint8_t RoomList::objectId() const
{
	return 21;
}

std::string RoomList::toString() const
{
	return "";
}

std::string RoomList::json() const
{
	return "";
}
} /* namespace CommNs */
