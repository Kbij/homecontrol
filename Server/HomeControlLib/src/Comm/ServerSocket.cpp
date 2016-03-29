/*
 * ServerSocket.cpp
 *
 *  Created on: Mar 28, 2016
 *      Author: koen
 */

#include "ServerSocket.h"
#include <glog/logging.h>
#include "boost/bind.hpp"
#include "ClientSocket.h"

namespace CommNs
{

ServerSocket::ServerSocket(int port):
	mIoService(),
	mAcceptor(mIoService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	mClients(),
	mServerThreadRunning(false),
	mServerThread(nullptr),
	mDataMutex()
{
	VLOG(1) << "Server socket created";
	ClientSocket* clientSocket = new ClientSocket(mIoService);
	mClients.push_back(clientSocket);
	mAcceptor.async_accept(clientSocket->socket(), boost::bind(&ServerSocket::handleAccept, this, clientSocket, boost::asio::placeholders::error));
	startServerThread();
}

ServerSocket::~ServerSocket()
{
	stopServerThread();
}

void ServerSocket::handleAccept(ClientSocketIf* clientSocket, const boost::system::error_code& error)
{
	if (!error)
	{
		VLOG(1) << "Connection accepted, staring listener";
		clientSocket->start();
		ClientSocket* client = new ClientSocket(mIoService);
		mClients.push_back(client);
		mAcceptor.async_accept(client->socket(), boost::bind(&ServerSocket::handleAccept, this, client,boost::asio::placeholders::error));
	}
}

void ServerSocket::startServerThread()
{
	if (!mServerThreadRunning)
	{
		mServerThreadRunning = true;
		mServerThread = new std::thread(&ServerSocket::serverThread, this);
	}
}

void ServerSocket::stopServerThread()
{
	mIoService.stop();
	for(ClientSocketIf* client: mClients)
	{
		client->close();
	}

	mServerThreadRunning = false;

    if (mServerThread)
    {
    	mServerThread->join();
    	delete mServerThread;
    	mServerThread = nullptr;
    }
}

void ServerSocket::serverThread()
{
	mIoService.run();
	VLOG(1) << "Server thread exiting";
}
} /* namespace CommNs */
