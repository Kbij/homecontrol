/*
 * Client.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "Client.h"
#include "ClientSocketIf.h"

namespace CommNs {

Client::Client(ClientSocketIf* clientSocket, ClientListenerIf* clientListener):
	mClientSocket(clientSocket),
	mClientListener(clientListener)
{

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

}


} /* namespace CommNs */
