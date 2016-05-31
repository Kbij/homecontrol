/*
 * SerialListenerIf.h
 *
 *  Created on: May 30, 2016
 *      Author: koen
 */

#ifndef COMM_SERIALLISTENERIF_H_
#define COMM_SERIALLISTENERIF_H_
#include <string>

namespace CommNs {

class SerialListenerIf
{
public:
	virtual ~SerialListenerIf() {};

	virtual void receiveLine(const std::string& line) = 0;
};

} /* namespace CommNs */



#endif /* COMM_SERIALLISTENERIF_H_ */
