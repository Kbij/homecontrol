/*
 * RoomTemperature.h
 *
 *  Created on: Jun 8, 2016
 *      Author: koen
 */

#ifndef COMMOBJECTS_ROOMTEMPERATURE_H_
#define COMMOBJECTS_ROOMTEMPERATURE_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>


namespace CommNs {

class RoomTemperature: public CommObjectIf
{
public:
	RoomTemperature(const std::string& roomId, double temperature);
	RoomTemperature(const std::string& json);
	virtual ~RoomTemperature();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;

	double temperature() const {return mRoomTemperature;};

private:
	std::string mRoomId;
	double mRoomTemperature;
};

} /* namespace CommNs */


#endif /* COMMOBJECTS_ROOMTEMPERATURE_H_ */
