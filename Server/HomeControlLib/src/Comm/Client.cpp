/*
 * Client.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "Client.h"
#include "ClientSocketIf.h"
#include "ClientListenerIf.h"
#include "CommObjects/CommObjectIf.h"
#include "CommObjects/ObjectFactory.h"
#include <stdint.h>
#include <glog/logging.h>

namespace
{
const std::string SERVERNAME = "HCM SERVER";

const int OBJ_KEEPALIVE = 0;
const int OBJ_HCNAME = 1;
const int OBJ_SERVERNAME = 2;
}

namespace CommNs {

Client::Client(ClientSocketIf* clientSocket, ClientListenerIf* clientListener):
	mClientSocket(clientSocket),
	mClientListener(clientListener),
	mName(),
	mConnectionState(ConnectionState::Initiated)
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
		switch(mConnectionState)
		{
			case ConnectionState::Initiated:
			{
				mName = std::string(frame.begin(), frame.end());
				LOG(INFO) << "Received client name: " << mName;
				mClientSocket->sendFrame(OBJ_SERVERNAME, {SERVERNAME.begin(), SERVERNAME.end()});
				mConnectionState = ConnectionState::Connected;
				break;
			}
			case ConnectionState::Connected:
			{
				std::string json(frame.begin(), frame.end());
				CommObjectIf* object = ObjectFactory::createObject(objectId, json);
				if (object)
				{
					mClientListener->receiveObject(mName, *object);
					delete object;
				}
				else
				{
					LOG(ERROR) << "Unable to create object, objectId: " << (int) objectId;
				}
				break;
			}
			default:
			{

			}
		}

	}
}


} /* namespace CommNs */
