/*
 * LocationInterval.h
 *
 *  Created on: Dec 25, 2018
 *      Author: koen
 */

#ifndef COMMOBJECTS_LOCATIONINTERVAL_H_
#define COMMOBJECTS_LOCATIONINTERVAL_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>

namespace CommNs
{

class LocationInterval: public CommObjectIf
{
public:
	LocationInterval(int interval);
	virtual ~LocationInterval();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;
private:
	int mInterval;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_LOCATIONINTERVAL_H_ */
