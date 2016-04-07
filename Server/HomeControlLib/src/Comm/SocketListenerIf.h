/*
 * SocketListenerIf.h
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#ifndef COMM_SOCKETLISTENERIF_H_
#define COMM_SOCKETLISTENERIF_H_
#include <vector>
#include <stdint.h>
#include <string>

namespace CommNs {

class SocketListenerIf
{
public:
	virtual ~SocketListenerIf() {};
	virtual void receiveFrame(uint8_t objectId, const std::vector<uint8_t>& frame) = 0;
	virtual std::string name() const = 0;
	virtual void socketClosed() = 0;
};

} /* namespace CommNs */



#endif /* COMM_SOCKETLISTENERIF_H_ */
