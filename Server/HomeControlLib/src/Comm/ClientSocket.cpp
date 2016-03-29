/*
 * Client.cpp
 *
 *  Created on: Mar 29, 2016
 *      Author: koen
 */

#include "ClientSocket.h"

#include <iostream>
#include <boost/bind.hpp>
#include <glog/logging.h>
#include <boost/system/error_code.hpp>

namespace CommNs {

ClientSocket::ClientSocket(boost::asio::io_service& ioService):
	mSocket(ioService),
	mBuffer()
{

}

ClientSocket::~ClientSocket()
{
}

boost::asio::ip::tcp::tcp::socket& ClientSocket::socket()
{
	return mSocket;
}

void ClientSocket::start()
{
	mSocket.async_read_some(boost::asio::buffer(mBuffer), boost::bind(&ClientSocket::handleRead, this,
							boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void ClientSocket::close()
{
	if (mSocket.is_open())
	{
		VLOG(1) << "Closing client socket";
		mSocket.close();
	}
}


void ClientSocket::handleRead(const boost::system::error_code& error, size_t bytesTransferred)
{
	if (!error)
	{
		VLOG(1) << "Bytes received: " << (int) bytesTransferred;

		mSocket.async_read_some(boost::asio::buffer(mBuffer), boost::bind(&ClientSocket::handleRead, this,
								boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		VLOG(1) << "Client socket error: " << error.category().name() << ':' << error.value();
	}
}
} /* namespace CommNs */
