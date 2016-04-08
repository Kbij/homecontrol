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

const int CONNECTING_TIMEOUT_MS = 1000;
const int RECEIVE_TIMEOUT_MS = 2 * 60 * 1000; // 2 Minutes
}

namespace CommNs {

Client::Client(ClientSocketIf* clientSocket, ClientListenerIf* clientListener):
	mClientSocket(clientSocket),
	mClientListener(clientListener),
	mName("Unknown"),
	mConnectionState(ConnectionState::Idle),
	mConnectingTime(0),
	mLastFrameTime(0)

{
	VLOG(1) << "Client created";
	mClientSocket->registerSocketListener(this);
}

Client::~Client()
{
	mClientSocket->close();
	delete mClientSocket;
}

boost::asio::ip::tcp::tcp::socket& Client::socket()
{
	return mClientSocket->socket();
}

void Client::start()
{
	mConnectionState = ConnectionState::Connecting;
	mClientSocket->start();
}

void Client::socketClosed()
{
	mConnectionState = ConnectionState::Disconnected;
}

void Client::receiveFrame(uint8_t objectId, const std::vector<uint8_t>& frame)
{
	VLOG(1) << "Frame received, size: " << frame.size();
	if (mClientListener)
	{
		switch(mConnectionState)
		{
			case ConnectionState::Connecting:
			{
				mName = std::string(frame.begin(), frame.end());
				LOG(INFO) << "Received client name: " << mName;
				mClientSocket->sendFrame(OBJ_SERVERNAME, {SERVERNAME.begin(), SERVERNAME.end()});
				mConnectionState = ConnectionState::Connected;
				mConnectingTime = 0;
				break;
			}
			case ConnectionState::Connected:
			{
				mLastFrameTime = 0;
				std::string json(frame.begin(), frame.end());
				CommObjectIf* object = ObjectFactory::createObject(objectId, json);
				if (object)
				{
					mClientListener->receiveObject(mName, object);
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

bool Client::isInactive(int milliSecondsPassed)
{
	switch(mConnectionState)
	{
		case ConnectionState::Idle:
		{
			return false;
			break;
		}
		case ConnectionState::Connecting:
		{
			mConnectingTime += milliSecondsPassed;
			return mConnectingTime >= CONNECTING_TIMEOUT_MS;
			break;
		}
		case ConnectionState::Connected:
		{
			mLastFrameTime += milliSecondsPassed;
			return mLastFrameTime >= RECEIVE_TIMEOUT_MS;
			break;
		}
		default:
		{
			return true;
		}
	}

	return true;
}

} /* namespace CommNs */
