/*
 * Client.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "Client.h"
#include "ClientSocketIf.h"

namespace CommNs {

Client::Client(ClientSocketIf* clientSocket):
	mClientSocket(clientSocket)
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
} /* namespace CommNs */
