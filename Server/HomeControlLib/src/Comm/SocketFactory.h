/*
 * SocketFactory.h
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#ifndef COMM_SOCKETFACTORY_H_
#define COMM_SOCKETFACTORY_H_
#include "SocketFactoryIf.h"
#include <boost/shared_ptr.hpp>

namespace CommNs
{

class SocketFactory: public SocketFactoryIf
{
public:
	SocketFactory();
	virtual ~SocketFactory();

	ServerSocketIf* createServerSocket(Server* server, int port);
	boost::shared_ptr<ClientSocketIf> createClientSocket();
private:
	boost::asio::io_service mIoService;
};

} /* namespace CommNs */

#endif /* COMM_SOCKETFACTORY_H_ */
