/*
 * Server.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "Server.h"
#include "Client.h"
#include "ServerSocketIf.h"
#include "SocketFactoryIf.h"

namespace CommNs {

Server::Server(SocketFactoryIf* factory, int port):
	mSocketFactory(factory),
	mServerSocket(factory->createServerSocket(this, port))
{

}

Server::~Server()
{
	delete mServerSocket;
}

Client* Server::newClient()
{
	return new Client(mSocketFactory->createClientSocket());
}

void Server::receiveObject(const CommObjectIf& object)
{

}


} /* namespace CommNs */
