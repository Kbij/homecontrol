/*
 * KeepAlive.h
 *
 *  Created on: Apr 23, 2016
 *      Author: koen
 */

#ifndef COMMOBJECTS_KEEPALIVE_H_
#define COMMOBJECTS_KEEPALIVE_H_
#include "CommObjectIf.h"
#include <string>
#include <ctime>

namespace CommNs
{

class KeepAlive: public CommObjectIf
{
public:
	KeepAlive();
	virtual ~KeepAlive();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const {return "";};

	time_t timeStamp() const {return mTimeStamp;};

private:
	std::time_t mTimeStamp;

};

} /* namespace CommNs */

#endif /* COMMOBJECTS_KEEPALIVE_H_ */
