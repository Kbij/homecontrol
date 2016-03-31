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
#include "ClientSocketIf.h"

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
	Client* client = new Client(mSocketFactory->createClientSocket(), this);
	ClientSocketIf* clientSocket = mSocketFactory->createClientSocket();
	clientSocket->registerSocketListener(client);
	return client;
}

void Server::receiveObject(const CommObjectIf& object)
{

}


} /* namespace CommNs */
