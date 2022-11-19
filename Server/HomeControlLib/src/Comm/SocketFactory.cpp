/*
 * SocketFactory.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "SocketFactory.h"
#include "ServerSocket.h"
#include "ClientSocket.h"
#include <memory>

namespace CommNs {

SocketFactory::SocketFactory():
	mIoService()
{
}

SocketFactory::~SocketFactory()
{
}

ServerSocketIf* SocketFactory::createServerSocket(Server* server, int port)
{
	return new ServerSocket(mIoService, server, port);
}

std::shared_ptr<ClientSocketIf> SocketFactory::createClientSocket()
{
	return std::make_shared<ClientSocket>(mIoService);
}

} /* namespace CommNs */
