/*
 * ObjectFactory.cpp
 *
 *  Created on: Apr 4, 2016
 *      Author: koen
 */

#include "ObjectFactory.h"
#include "GpsLocation.h"
#include "MessageObject.h"

namespace CommNs {

ObjectFactory::ObjectFactory()
{
}

ObjectFactory::~ObjectFactory()
{
}

CommObjectIf* ObjectFactory::createObject(uint8_t objectId, const std::string& json)
{

	switch(objectId)
	{
		case 10: return new GpsLocation(json);
		case 11: return new MessageObject(json);
		default: return nullptr;
	}

	return nullptr;
}
} /* namespace CommNs */
