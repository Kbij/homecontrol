/*
 * HeaterListenerIf.h
 *
 *  Created on: Aug 11, 2016
 *      Author: koen
 */

#ifndef LOGIC_HEATERLISTENERIF_H_
#define LOGIC_HEATERLISTENERIF_H_

#include <string>

namespace LogicNs {

class HeaterListenerIf
{
public:
	virtual ~HeaterListenerIf() {};

	virtual void heaterOn(const std::string& roomId) = 0;
	virtual void heaterOff(const std::string& roomId) = 0;
};

} /* namespace LogicNs */



#endif /* LOGIC_HEATERLISTENERIF_H_ */
