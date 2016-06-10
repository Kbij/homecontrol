/*
 * CommObjectIf.h
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#ifndef COMM_COMMOBJECTIF_H_
#define COMM_COMMOBJECTIF_H_
#include <stdint.h>
#include <string>

namespace CommNs {

class CommObjectIf
{
public:
	virtual ~CommObjectIf() {};
	virtual uint8_t objectId() const = 0;
	virtual std::string toString() const = 0;

	virtual std::string json() const = 0;
};

} /* namespace CommNs */


#endif /* COMM_COMMOBJECTIF_H_ */
