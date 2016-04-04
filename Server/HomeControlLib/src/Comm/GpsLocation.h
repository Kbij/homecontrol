/*
 * GpsLocation.h
 *
 *  Created on: Apr 4, 2016
 *      Author: koen
 */

#ifndef COMM_GPSLOCATION_H_
#define COMM_GPSLOCATION_H_
#include "CommObjectIf.h"
#include <string>

namespace CommNs {

class GpsLocation: public CommObjectIf
{
public:
	GpsLocation(const std::string& json);
	virtual ~GpsLocation();

	uint8_t objectId() const;
};

} /* namespace CommNs */

#endif /* COMM_GPSLOCATION_H_ */
