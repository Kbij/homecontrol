/*
 * ObjectFactory.h
 *
 *  Created on: Apr 4, 2016
 *      Author: koen
 */

#ifndef COMM_OBJECTFACTORY_H_
#define COMM_OBJECTFACTORY_H_
#include <string>
#include <stdint.h>


namespace CommNs {
class CommObjectIf;

class ObjectFactory {
public:
	ObjectFactory();
	virtual ~ObjectFactory();

	static CommObjectIf* createObject(uint8_t objectId, const std::string& json);
};

} /* namespace CommNs */

#endif /* COMM_OBJECTFACTORY_H_ */
