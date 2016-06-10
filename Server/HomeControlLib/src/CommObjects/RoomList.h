/*
 * RoomList.h
 *
 *  Created on: Jun 10, 2016
 *      Author: koen
 */

#ifndef COMMOBJECTS_ROOMLIST_H_
#define COMMOBJECTS_ROOMLIST_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>


namespace CommNs {

class RoomList: public CommObjectIf
{
public:
	RoomList();
	RoomList(const std::string& json);
	virtual ~RoomList();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;

//	double temperature() const {return mRoomTemperature;};

private:
//	double mRoomTemperature;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_ROOMLIST_H_ */
