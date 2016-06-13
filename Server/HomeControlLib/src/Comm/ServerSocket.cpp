/*
 * ServerSocket.cpp
 *
 *  Created on: Mar 28, 2016
 *      Author: koen
 */

#include "ServerSocket.h"
#include "Server.h"
#include "Client.h"
#include <glog/logging.h>
#include "boost/bind.hpp"
#include "ClientSocket.h"

namespace CommNs
{

ServerSocket::ServerSocket(boost::asio::io_service& ioService, Server* server, int port):
	mIoService(ioService),
	mServer(server),
	mAcceptor(mIoService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	mServerThreadRunning(false),
	mServerThread(nullptr),
	mDataMutex()
{
	VLOG(1) << "Server socket created";
	Client* client = mServer->newClient();
	mAcceptor.async_accept(client->socket(), boost::bind(&ServerSocket::handleAccept, this, client, boost::asio::placeholders::error));
	startServerThread();
}

ServerSocket::~ServerSocket()
{
	stopServerThread();
}

void ServerSocket::handleAccept(Client* client, const boost::system::error_code& error)
{
	if (!error)
	{
		VLOG(1) << "Connection accepted, staring listener";
		client->start();
		Client* client = mServer->newClient();
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
//	for(ClientSocketIf* client: mClients)
//	{
//		client->close();
//	}

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
	LOG(INFO) << "Server socket thread exiting";
}
} /* namespace CommNs */
