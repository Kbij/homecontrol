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
#include "ClientListenerIf.h"
#include "CommListenerIf.h"

namespace CommNs {

Server::Server(SocketFactoryIf* factory, int port):
	mSocketFactory(factory),
	mServerSocket(factory->createServerSocket(this, port)),
	mCommListeners()
{

}

Server::~Server()
{
	delete mServerSocket;
}

void Server::registerCommListener(CommListenerIf* listener)
{
	if (listener)
	{
		std::lock_guard<std::mutex> lock(mDataMutex);
		mCommListeners.insert(listener);
	}
}

void Server::unRegisterCommListener(CommListenerIf* listener)
{
	if (listener)
	{
		std::lock_guard<std::mutex> lock(mDataMutex);
		mCommListeners.erase(listener);
	}
}

Client* Server::newClient()
{
	Client* client = new Client(mSocketFactory->createClientSocket(), this);
	return client;
}

void Server::receiveObject(const std::string name, const CommObjectIf& object)
{
	std::lock_guard<std::mutex> lock(mDataMutex);
	for(const auto& listener: mCommListeners)
	{
		listener->receiveObject(name, object);
	}
}


} /* namespace CommNs */
