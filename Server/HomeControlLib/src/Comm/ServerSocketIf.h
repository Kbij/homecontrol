/*
 * ServerSocketIf.h
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#ifndef COMM_SERVERSOCKETIF_H_
#define COMM_SERVERSOCKETIF_H_

namespace CommNs {

class ServerSocketIf
{
public:
	virtual ~ServerSocketIf() {};

	virtual void close() = 0;
};

} /* namespace CommNs */



#endif /* COMM_SERVERSOCKETIF_H_ */
