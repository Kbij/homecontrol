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
#include <iostream>

namespace
{
const int HEADER_TOTAL_LENGTH = 6;
const int HEADER_DATA_LENGTH_POSITION = 3;
const int HEADER_OBJECTID_POSITION = 5;

const std::vector<uint8_t> HC_HEADER = {'H', 'C', 'M'};

}
namespace CommNs {

ClientSocket::ClientSocket(boost::asio::io_service& ioService):
	mSocket(ioService),
	mSocketBuffer(),
	mReceiveBuffer(),
	mSocketListener(nullptr)
{

}

ClientSocket::~ClientSocket()
{
	if (mSocket.is_open())
	{
		VLOG(1) << "Closing client socket";
		mSocket.close();
	}
}

boost::asio::ip::tcp::tcp::socket& ClientSocket::socket()
{
	return mSocket;
}

void ClientSocket::start()
{
	mSocket.async_read_some(boost::asio::buffer(mSocketBuffer), boost::bind(&ClientSocket::handleRead, this,
							boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void ClientSocket::registerSocketListener(SocketListenerIf* socketListener)
{
	mSocketListener = socketListener;
}

void ClientSocket::unRegisterSocketListener()
{
	mSocketListener = nullptr;
}

void ClientSocket::sendFrame(uint8_t objectId, const std::vector<uint8_t>& frame)
{
	if (mSocket.is_open())
	{
		VLOG(2) << "Send frame, length: " << frame.size() << ",objectId: " << (int) objectId;
		std::vector<uint8_t> dataFrame(HC_HEADER.begin(), HC_HEADER.end());

        int length = frame.size();
        uint8_t msb = (uint8_t)(length / 256);
		uint8_t lsb = length - (msb * 256);
		dataFrame.push_back(msb);
		dataFrame.push_back(lsb);
		dataFrame.push_back(objectId);
		dataFrame.insert(dataFrame.end(), frame.begin(), frame.end());
		boost::system::error_code error;
		boost::asio::write(mSocket, boost::asio::buffer(dataFrame), boost::asio::transfer_all(), error);
	}
}

void ClientSocket::handleRead(const boost::system::error_code& error, size_t bytesTransferred)
{
	if (!error)
	{
		VLOG(2) << "Bytes received: " << (int) bytesTransferred;
		mReceiveBuffer.insert(mReceiveBuffer.end(), mSocketBuffer.begin(), mSocketBuffer.begin() + bytesTransferred);
		processBuffer();

		mSocket.async_read_some(boost::asio::buffer(mSocketBuffer), boost::bind(&ClientSocket::handleRead, this,
								boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		LOG(ERROR) << "Client socket error: " << error.category().name() << ':' << error.value();
		if (mSocketListener)
		{
			mSocketListener->socketClosed();
		}
	}
}

void ClientSocket::processBuffer()
{
	bool packetFound = true;
	while (packetFound && mReceiveBuffer.size() > 0)
	{
		packetFound = false;
		std::vector<uint8_t> hcmData;

		if (mReceiveBuffer.size() < (size_t) HEADER_TOTAL_LENGTH)
		{ // Not received the full buffer; return
			VLOG(3) << "Not a full header received, buffer size: " << mReceiveBuffer.size();
			return;
		}

		auto itStartPosition = std::search(mReceiveBuffer.begin(), mReceiveBuffer.end(), HC_HEADER.begin(), HC_HEADER.end());
		if (itStartPosition == mReceiveBuffer.end())
		{ //HCB string not found; return
			if (mReceiveBuffer.size() > 1500)
			{	//Avoid a buffer build-up
				LOG(ERROR) << "While searching for HCM string, buffer cleared. Size: " << mReceiveBuffer.size();
				mReceiveBuffer.clear();
			}
			return;
		}
		auto startPosition = itStartPosition - mReceiveBuffer.begin();
		// We found a full HCB header; we can start processing it
		int dataLength = mReceiveBuffer[startPosition + HEADER_DATA_LENGTH_POSITION] * 256 +
						 mReceiveBuffer[startPosition + HEADER_DATA_LENGTH_POSITION + 1];
		uint8_t objectId = mReceiveBuffer[HEADER_OBJECTID_POSITION];

		if (mReceiveBuffer.size() < (size_t) dataLength + HEADER_TOTAL_LENGTH)
		{ // Not all data received
			VLOG(3) << "Not all data received, buffer size: " << mReceiveBuffer.size() << ", data length: " << dataLength;
			return;
		}

		VLOG(3) << "Full packet received. buffer size: " << mReceiveBuffer.size() << ", packet length: " << dataLength;
		// All data received; send it to the CloudComm, and cleanup our buffer
		hcmData.insert(hcmData.end(), itStartPosition +  HEADER_TOTAL_LENGTH, itStartPosition + HEADER_TOTAL_LENGTH + dataLength);
		mReceiveBuffer.erase(mReceiveBuffer.begin(), itStartPosition + HEADER_TOTAL_LENGTH + dataLength);
		packetFound = true;
		VLOG(3) << "Buffer size: " << mReceiveBuffer.size();
		if (mSocketListener)
		{
			VLOG(2) << "Payload received, length: " << hcmData.size();
			mSocketListener->receiveFrame(objectId, hcmData);
		}
	}
}
} /* namespace CommNs */
