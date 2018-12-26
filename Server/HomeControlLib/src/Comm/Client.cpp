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
const int RECEIVE_TIMEOUT_MS = 5 * 60 * 1000; // 2 Minutes
}

namespace CommNs {

Client::Client(ClientSocketIf* clientSocket, ClientListenerIf* clientListener):
	mClientSocket(clientSocket),
	mClientListener(clientListener),
	mName("Unknown"),
	mConnectionState(ConnectionState::Idle),
	mConnectingTime(0),
	mLastFrameTime(0),
	mLocationInterval(0)

{
	VLOG(1) << "[" << this << "] Client created";
	mClientSocket->registerSocketListener(this);
}

Client::~Client()
{
	mClientSocket->unRegisterSocketListener();
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
			if (mConnectingTime >= CONNECTING_TIMEOUT_MS)
			{
				VLOG(2) << "[" << this << "][" << mName << "] Client inactive: mConnectingTime=" << mConnectingTime;
			}
			return mConnectingTime >= CONNECTING_TIMEOUT_MS;
			break;
		}
		case ConnectionState::Connected:
		{
			mLastFrameTime += milliSecondsPassed;
			if (mLastFrameTime >= RECEIVE_TIMEOUT_MS)
			{
				VLOG(2) << "[" << this << "][" << mName << "] mLastFrameTime: " << mLastFrameTime;
			}
			return mLastFrameTime >= RECEIVE_TIMEOUT_MS;
			break;
		}
		default:
		{
			VLOG(2) << "[" << this << "][" << mName << "] Client inactive: default, mConnectionState:" << (int) mConnectionState;
			return true;
		}
	}

	VLOG(2) << "[" << this << "][" << mName << "] Client inactive: should not happen";
	return true;
}

void Client::sendFrame(uint8_t objectId, const std::vector<uint8_t>& frame)
{
	VLOG(2) << "[" << this << "][" << mName << "] Send objectId: " << (int) objectId;
	if (mConnectionState == ConnectionState::Connected)
	{
		mClientSocket->sendFrame(objectId, frame);
	}
}

int Client::locationInterval()
{
	return mLocationInterval;
}

void Client::locationInterval(int interval)
{
	mLocationInterval = interval;
}

void Client::receiveFrame(uint8_t objectId, const std::vector<uint8_t>& frame)
{
	VLOG(2) << "[" << this << "][" << mName << "] Frame received, size: " << frame.size() << ", objectId: " << (int) objectId;
	if (mClientListener)
	{
		switch(mConnectionState)
		{
			case ConnectionState::Connecting:
			{
				mName = std::string(frame.begin(), frame.end());
				LOG(INFO) << "[" << this << "] Received client name: " << mName;
				mClientSocket->name(mName);
				mClientSocket->sendFrame(OBJ_SERVERNAME, {SERVERNAME.begin(), SERVERNAME.end()});
				mConnectionState = ConnectionState::Connected;
				mConnectingTime = 0;
				if (mClientListener)
				{
					mClientListener->clientAuthenticated(this, mName);
				}
				break;
			}
			case ConnectionState::Connected:
			{
				mLastFrameTime = 0;
				std::string json(frame.begin(), frame.end());
				CommObjectIf* object = ObjectFactory::createObject(objectId, json);
				if (object)
				{
					VLOG(2) << "[" << this << "][" << mName << "] Object received, objectId: " << (int) objectId;
					mClientListener->receiveObject(mName, object);

					if (objectId == 0)
					{// If keep alive; send it back
						mClientSocket->sendFrame(0, frame);
					}
					delete object;
				}
				else
				{
					LOG(ERROR) << "[" << this << "][" << mName << "] Unable to create object, objectId: " << (int) objectId;
				}
				break;
			}
			default:
			{

			}
		}

	}
}

void Client::socketClosed()
{
	VLOG(2) << "[" << this << "][" << mName << "] socketCloded";
	mConnectionState = ConnectionState::Disconnected;
}

} /* namespace CommNs */
