/*
 * SerialListenerIf.h
 *
 *  Created on: May 30, 2016
 *      Author: koen
 */

#ifndef COMM_SERIALLISTENERIF_H_
#define COMM_SERIALLISTENERIF_H_
#include <string>
#include <vector>

namespace CommNs {

class SerialListenerIf
{
public:
	virtual ~SerialListenerIf() {};

	virtual void receiveData(const std::vector<uint8_t>& data) = 0;
};

} /* namespace CommNs */



#endif /* COMM_SERIALLISTENERIF_H_ */
