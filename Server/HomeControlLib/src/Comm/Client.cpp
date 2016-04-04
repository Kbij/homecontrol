/*
 * Client.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "Client.h"
#include "ClientSocketIf.h"
#include "ClientListenerIf.h"
#include "CommObjectIf.h"
#include "ObjectFactory.h"
#include <stdint.h>
#include <glog/logging.h>

namespace CommNs {

Client::Client(ClientSocketIf* clientSocket, ClientListenerIf* clientListener):
	mClientSocket(clientSocket),
	mClientListener(clientListener)
{
	mClientSocket->registerSocketListener(this);
}

Client::~Client()
{
}

boost::asio::ip::tcp::tcp::socket& Client::socket()
{
	return mClientSocket->socket();
}

void Client::start()
{
	mClientSocket->start();
}

void Client::receiveFrame(uint8_t objectId, const std::vector<uint8_t>& frame)
{
	VLOG(1) << "Frame received, size: " << frame.size();
	if (mClientListener)
	{
		std::string json(frame.begin(), frame.end());
		CommObjectIf* object = ObjectFactory::createObject(objectId, json);
		if (object)
		{
			std::cout << "object created" << std::endl;
			mClientListener->receiveObject(*object);
			delete object;
		}
		else
		{
			LOG(ERROR) << "Unable to create object, objectId: " << objectId;
		}
	}
}


} /* namespace CommNs */
