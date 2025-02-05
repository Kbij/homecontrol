/*
 * ObjectPrinter.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: koen
 */

#include "ObjectPrinter.h"
#include "CommObjects/CommObjectIf.h"
#include "glog/logging.h"

namespace LogicNs {

ObjectPrinter::ObjectPrinter()
{
}

ObjectPrinter::~ObjectPrinter()
{
}

void ObjectPrinter::clientConnected(const std::string& name)
{
	LOG(INFO) << "Client connected: " << name;
}

void ObjectPrinter::clientDisConnected(const std::string& name)
{
	LOG(INFO) << "Client disconnected: " << name;
}

void ObjectPrinter::receiveObject(const std::string name, const CommNs::CommObjectIf* object)
{
	if (object)
	{
		LOG(INFO) << "Name: " << name << ", Object: " << object->toString();
	}
}


} /* namespace LogicNs */
