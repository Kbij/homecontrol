/*
 * ObjectFactory.cpp
 *
 *  Created on: Apr 4, 2016
 *      Author: koen
 */

#include "ObjectFactory.h"
#include "GpsLocation.h"
#include "KeepAlive.h"
#include "MessageObject.h"
#include "RoomTemperature.h"
#include "RoomList.h"
#include "TemperatureMonitoring.h"
#include "TemperatureUp.h"
#include "TemperatureDown.h"

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
		case 0: return new KeepAlive();
		case 10: return new GpsLocation(json);
		case 11: return new MessageObject(json);

		// Will not receive this; only send
		case 20: return new TemperatureMonitoring(json);
		case 21: return new RoomList(json);
		case 22: return new RoomTemperature(json);

		case 30: return new TemperatureUp(json);
		case 31: return new TemperatureDown(json);
		default: return nullptr;
	}

	return nullptr;
}
} /* namespace CommNs */
