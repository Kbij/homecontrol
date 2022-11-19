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
#include "ClientSocket.h"
#include <boost/bind/bind.hpp>

using namespace boost::placeholders;

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
	try
	{
		mIoService.run();
	}
	catch(const std::exception& ex)
	{
		LOG(ERROR) << "Exception in ServerThread: " << ex.what();
	}

	LOG(INFO) << "Server socket thread exiting";
}
} /* namespace CommNs */
