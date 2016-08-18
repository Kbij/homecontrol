/*
 * RoomListenerIf.h
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#ifndef LOGIC_ROOMLISTENERIF_H_
#define LOGIC_ROOMLISTENERIF_H_
#include <string>

namespace LogicNs {

class RoomListenerIf
{
public:
	virtual ~RoomListenerIf() {};

	virtual void temperatureChanged(const std::string& roomId, double temperature) = 0;
	virtual void setPointChanged(const std::string& roomId, double setTemperature) = 0;

	virtual void heaterOn(const std::string& roomId) = 0;
	virtual void heaterOff(const std::string& roomId) = 0;
};

} /* namespace LogicNs */



#endif /* LOGIC_ROOMLISTENERIF_H_ */
