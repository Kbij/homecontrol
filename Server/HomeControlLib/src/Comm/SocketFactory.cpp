/*
 * SocketFactory.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "SocketFactory.h"
#include "ServerSocket.h"
#include "ClientSocket.h"

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

ClientSocketIf* SocketFactory::createClientSocket()
{
	return new ClientSocket(mIoService);
}

} /* namespace CommNs */
