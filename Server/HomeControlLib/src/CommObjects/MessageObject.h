/*
 * MessageObject.h
 *
 *  Created on: Apr 7, 2016
 *      Author: koen
 */

#ifndef COMMOBJECTS_MESSAGEOBJECT_H_
#define COMMOBJECTS_MESSAGEOBJECT_H_
#include "CommObjectIf.h"
#include <string>
#include <ctime>

namespace CommNs {

class MessageObject: public CommObjectIf
{
public:
	MessageObject(const std::string& json);
	virtual ~MessageObject();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;

	std::string message() const {return mMessage;};
	time_t timeStamp() const {return mTimeStamp;};

private:
	std::string mMessage;
	std::time_t mTimeStamp;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_MESSAGEOBJECT_H_ */
