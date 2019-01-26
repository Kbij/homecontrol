/*
 * RoomTemperature.h
 *
 *  Created on: Jun 8, 2016
 *      Author: koen
 */

#ifndef COMMOBJECTS_SETTEMPERATURE_H_
#define COMMOBJECTS_SETTEMPERATURE_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>


namespace CommNs {

class SetTemperature: public CommObjectIf
{
public:
	SetTemperature(const std::string& roomId, double temperature);
	SetTemperature(const std::string& json);
	virtual ~SetTemperature();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;

	double temperature() const {return mSetTemperature;};

private:
	std::string mRoomId;
	double mSetTemperature;
};

} /* namespace CommNs */


#endif /* COMMOBJECTS_SETTEMPERATURE_H_ */
