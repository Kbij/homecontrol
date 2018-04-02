/*
 * TimeListenerIf.h
 *
 *  Created on: Apr 1, 2018
 *      Author: koen
 */

#ifndef COMM_TIMELISTENERIF_H_
#define COMM_TIMELISTENERIF_H_
#include <string>

namespace CommNs {

class TimeListenerIf
{
public:

	virtual void writeTime(const std::string& time) = 0;
};

} /* namespace CommNs */



#endif /* COMM_TIMELISTENERIF_H_ */
