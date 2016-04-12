/*
 * ObjectPrinter.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: koen
 */

#include "ObjectPrinter.h"
#include "CommObjects/CommObjectIf.h"
#include "glog/logging.h"

namespace CommNs {

ObjectPrinter::ObjectPrinter()
{
}

ObjectPrinter::~ObjectPrinter()
{
}

void ObjectPrinter::receiveObject(const std::string name, const CommObjectIf* object)
{
	if (object)
	{
		LOG(INFO) << "Name: " << name << ", Object: " << object->toString();
	}
}


} /* namespace CommNs */
