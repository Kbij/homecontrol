/*
 * TemperatureDown.h
 *
 *  Created on: Jan 26, 2019
 *      Author: koen
 */

#ifndef COMMOBJECTS_TEMPERATUREDOWN_H_
#define COMMOBJECTS_TEMPERATUREDOWN_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>

namespace CommNs {

class TemperatureDown: public CommObjectIf
{
public:
	TemperatureDown();
	TemperatureDown(const std::string& json);
	virtual ~TemperatureDown();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;

	std::string roomId() const {return mRoomId;};
private:
	std::string mRoomId;

};

} /* namespace CommNs */

#endif /* COMMOBJECTS_TEMPERATUREDOWN_H_ */
