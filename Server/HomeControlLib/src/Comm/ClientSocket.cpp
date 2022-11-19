/*
 * Client.cpp
 *
 *  Created on: Mar 29, 2016
 *      Author: koen
 */

#include "ClientSocket.h"

#include <iostream>
#include <glog/logging.h>
#include <boost/system/error_code.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
using namespace boost::placeholders;

namespace
{
const int HEADER_TOTAL_LENGTH = 6;
const int HEADER_DATA_LENGTH_POSITION = 3;
const int HEADER_OBJECTID_POSITION = 5;

const std::vector<uint8_t> HC_HEADER = {'H', 'C', 'M'};
}

namespace CommNs
{

ClientSocket::ClientSocket(boost::asio::io_service& ioService):
	mSocket(ioService),
	mSocketBuffer(),
	mReceiveBuffer(),
	mSendBuffer(),
	mSocketListener(nullptr),
	mName(),
	mLocalPort(),
	mDataMutex()
{
}

ClientSocket::~ClientSocket()
{
	if (mSocket.is_open())
	{
		mSocket.close();
		VLOG(5) << "[" << mName << ", " << mLocalPort << "] Socket destroyed";
	}
}

boost::asio::ip::tcp::tcp::socket& ClientSocket::socket()
{
	return mSocket;
}

void ClientSocket::start()
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	mSocket.async_read_some(boost::asio::buffer(mSocketBuffer), boost::bind(&ClientSocket::handleRead, shared_from_this(),
							boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

	mLocalPort = mSocket.remote_endpoint().port();
}

void ClientSocket::close()
{
	if (mSocket.is_open())
	{
		mSocket.close();
	}
}

void ClientSocket::registerSocketListener(SocketListenerIf* socketListener)
{
	VLOG(5) << "Socket listener: " << socketListener;
	std::lock_guard<std::mutex> lg(mDataMutex);
	mSocketListener = socketListener;
}

void ClientSocket::unRegisterSocketListener()
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	mSocketListener = nullptr;
}

void ClientSocket::sendFrame(uint8_t objectId, const std::vector<uint8_t>& frame)
{
	std::lock_guard<std::mutex> lg(mDataMutex);
	if (mSocket.is_open())
	{
		VLOG(5) << "[" << mName << ", " << mLocalPort << "] Send frame, length: " << frame.size() << ",objectId: " << (int) objectId;
		mSendBuffer.clear();
		mSendBuffer.insert(mSendBuffer.end(), HC_HEADER.begin(), HC_HEADER.end());
        int length = frame.size();
        uint8_t msb = (uint8_t)(length / 256);
		uint8_t lsb = length - (msb * 256);

		mSendBuffer.push_back(msb);
		mSendBuffer.push_back(lsb);
		mSendBuffer.push_back(objectId);
		mSendBuffer.insert(mSendBuffer.end(), frame.begin(), frame.end());
		boost::system::error_code error;
		mSocket.async_send(boost::asio::buffer(mSendBuffer.data(), mSendBuffer.size()), boost::bind(&ClientSocket::handleWrite, shared_from_this(),
				 	 	 	 	 	 	 	 	 	 	 	 boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
}

void ClientSocket::handleRead(const boost::system::error_code& error, size_t bytesTransferred)
{
	if (!error)
	{
		std::vector<ReceivedFrame> receivedFrames;
		{
			std::lock_guard<std::mutex> lg(mDataMutex);
			VLOG(5) << "[" << mName << ", " << mLocalPort << "] Bytes received: " << (int) bytesTransferred;
			mReceiveBuffer.insert(mReceiveBuffer.end(), mSocketBuffer.begin(), mSocketBuffer.begin() + bytesTransferred);
			receivedFrames = processBuffer();

			mSocket.async_read_some(boost::asio::buffer(mSocketBuffer), boost::bind(&ClientSocket::handleRead, shared_from_this(),
									boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}

		for(const auto& frame: receivedFrames)
		{
			if (mSocketListener)
			{
				VLOG(2) << "[" << mName << ", " << mLocalPort << "] Payload received, length: " << frame.Frame.size();
				mSocketListener->receiveFrame(frame.ObjectId, frame.Frame);
			}
		}
	}
	else
	{
		LOG(ERROR) << "[" << mName << ", " << mLocalPort << "] Socket read error, error code: " << error;
		if (mSocketListener)
		{
			mSocketListener->socketClosed();
		}
	}
}

void ClientSocket::handleWrite(const boost::system::error_code& error, std::size_t bytesTransferred)
{
	VLOG(5) << "Packet written";
}

std::vector<ReceivedFrame> ClientSocket::processBuffer()
{
	std::vector<ReceivedFrame> result;
	bool packetFound = true;
	while (packetFound && mReceiveBuffer.size() > 0)
	{
		packetFound = false;
		std::vector<uint8_t> hcmData;

		if (mReceiveBuffer.size() < (size_t) HEADER_TOTAL_LENGTH)
		{ // Not received the full buffer; return
			VLOG(5) << "[" << mName << ", " << mLocalPort << "] Not a full header received, buffer size: " << mReceiveBuffer.size();
			return result;
		}

		auto itStartPosition = std::search(mReceiveBuffer.begin(), mReceiveBuffer.end(), HC_HEADER.begin(), HC_HEADER.end());
		if (itStartPosition == mReceiveBuffer.end())
		{ //HCB string not found; return
			if (mReceiveBuffer.size() > 1500)
			{	//Avoid a buffer build-up
				LOG(ERROR) << "[" << mName << ", " << mLocalPort << "] While searching for HCM string, buffer cleared. Size: " << mReceiveBuffer.size();
				mReceiveBuffer.clear();
			}
			return result;
		}
		auto startPosition = itStartPosition - mReceiveBuffer.begin();
		// We found a full HCB header; we can start processing it
		int dataLength = mReceiveBuffer[startPosition + HEADER_DATA_LENGTH_POSITION] * 256 +
						 mReceiveBuffer[startPosition + HEADER_DATA_LENGTH_POSITION + 1];
		uint8_t objectId = mReceiveBuffer[HEADER_OBJECTID_POSITION];

		if (mReceiveBuffer.size() < (size_t) dataLength + HEADER_TOTAL_LENGTH)
		{ // Not all data received
			VLOG(5) << "[" << mName << ", " << mLocalPort << "] Not all data received, buffer size: " << mReceiveBuffer.size() << ", data length: " << dataLength;
			return result;
		}

		VLOG(5) << "[" << mName << ", " << mLocalPort << "] Full packet received. buffer size: " << mReceiveBuffer.size() << ", packet length: " << dataLength;
		// All data received; send it to the CloudComm, and cleanup our buffer
		hcmData.insert(hcmData.end(), itStartPosition +  HEADER_TOTAL_LENGTH, itStartPosition + HEADER_TOTAL_LENGTH + dataLength);
		mReceiveBuffer.erase(mReceiveBuffer.begin(), itStartPosition + HEADER_TOTAL_LENGTH + dataLength);
		packetFound = true;
		VLOG(5) << "[" << mName << ", " << mLocalPort << "] Buffer size: " << mReceiveBuffer.size();

		ReceivedFrame frame;
		frame.ObjectId = objectId;
		frame.Frame = hcmData;
		result.push_back(frame);
	}

	return result;
}
} /* namespace CommNs */
