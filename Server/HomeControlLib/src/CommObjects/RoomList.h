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
#include <vector>

namespace CommNs {
struct Room
{
	std::string RoomName;
	std::string RoomId;
	double RoomTemperature;
	double SetTemperature;
};

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

	void addRoom(Room room);

private:
	std::vector<Room> mRooms;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_ROOMLIST_H_ */
