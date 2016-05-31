/*
 * SerialIf.h
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */

#ifndef COMM_SERIALIF_H_
#define COMM_SERIALIF_H_
#include <string>

namespace CommNs {
class SerialListenerIf;

class SerialIf
{
public:
	virtual ~SerialIf() {};

	virtual void registerSerialListener(SerialListenerIf* listener) = 0;
	virtual void unRegisterSerialListener() = 0;

	virtual void writeLine(const std::string& line) = 0;
};

} /* namespace CommNs */



#endif /* COMM_SERIALIF_H_ */
