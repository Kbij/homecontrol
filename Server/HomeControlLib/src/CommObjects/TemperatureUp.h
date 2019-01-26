/*
 * TemperatureUp.h
 *
 *  Created on: Jan 26, 2019
 *      Author: koen
 */

#ifndef COMMOBJECTS_TEMPERATUREUP_H_
#define COMMOBJECTS_TEMPERATUREUP_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>

namespace CommNs {

class TemperatureUp: public CommObjectIf
{
public:
	TemperatureUp();
	TemperatureUp(const std::string& json);
	virtual ~TemperatureUp();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;

	std::string roomId() const {return mRoomId;};
private:
	std::string mRoomId;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_TEMPERATUREUP_H_ */
