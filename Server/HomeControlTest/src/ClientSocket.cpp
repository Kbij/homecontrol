/*
 * ClientSocket.cpp
 *
 *  Created on: Mar 31, 2016
 *      Author: koen
 */


#include "Comm/ClientSocket.h"
#include "Comm/SocketListenerIf.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>
#include <stdint.h>

class SocketListener: public CommNs::SocketListenerIf
{
public:
	SocketListener():
		mLastObjectId(),
		mLastReceivedFrame()
	{} ;
	void receiveFrame(uint8_t objectId, const std::vector<uint8_t>& frame) {mLastObjectId = objectId; mLastReceivedFrame = frame;};
	std::string name() const {return "Stub";};
	void socketClosed() {};
	uint8_t mLastObjectId;
	std::vector<uint8_t> mLastReceivedFrame;
};

TEST(ClientSocket, Constructor)
{

}

TEST(ClientSocket, Receive1FrameOnePart)
{
	SocketListener listener;
	boost::asio::io_service ioService;
	CommNs::ClientSocket* clientSocket = new CommNs::ClientSocket(ioService);
	clientSocket->registerSocketListener(&listener);
	boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 3000));

	//cloudSocket->connect("localhost", 3000);
	boost::asio::ip::tcp::socket sendSoc(ioService);
	boost::asio::ip::tcp::resolver resolver(ioService);
	boost::asio::ip::tcp::resolver::query query("localhost", "3000");
	boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);

	boost::asio::connect(sendSoc, it);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	acceptor.accept(clientSocket->socket());
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	clientSocket->start();

	//Data length = 4
	//Object ID = 0
	std::vector<uint8_t> myData = {'H', 'C', 'M', 0, 4, 0, 'T','E','S','T'};
	boost::system::error_code error;
	boost::asio::write(sendSoc, boost::asio::buffer(myData), boost::asio::transfer_all(), error);

	// Send the data; and let it be received by our socket under test
	ioService.run_one();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::vector<uint8_t> expected = {'T','E','S','T'};
	ASSERT_EQ(expected, listener.mLastReceivedFrame);
	ASSERT_EQ(0, listener.mLastObjectId);

	delete clientSocket;
}
TEST(ClientSocket, ReceiveKeepalive)
{
	SocketListener listener;
	boost::asio::io_service ioService;
	CommNs::ClientSocket* clientSocket = new CommNs::ClientSocket(ioService);
	clientSocket->registerSocketListener(&listener);
	boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 3000));

	//cloudSocket->connect("localhost", 3000);
	boost::asio::ip::tcp::socket sendSoc(ioService);
	boost::asio::ip::tcp::resolver resolver(ioService);
	boost::asio::ip::tcp::resolver::query query("localhost", "3000");
	boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);

	boost::asio::connect(sendSoc, it);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	acceptor.accept(clientSocket->socket());
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	clientSocket->start();

	//Data length = 0
	//Object ID = 0
	std::vector<uint8_t> myData = {'H', 'C', 'M', 0, 0, 0};
	boost::system::error_code error;
	boost::asio::write(sendSoc, boost::asio::buffer(myData), boost::asio::transfer_all(), error);

	// Send the data; and let it be received by our socket under test
	ioService.run_one();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::vector<uint8_t> expected = {};
	ASSERT_EQ(expected, listener.mLastReceivedFrame);
	ASSERT_EQ(0, listener.mLastObjectId);

	delete clientSocket;
}
TEST(ClientSocket, Receive1FrameTwoParts)
{
	SocketListener listener;
	boost::asio::io_service ioService;
	CommNs::ClientSocket* clientSocket = new CommNs::ClientSocket(ioService);
	clientSocket->registerSocketListener(&listener);
	boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 3000));

	//cloudSocket->connect("localhost", 3000);
	boost::asio::ip::tcp::socket sendSoc(ioService);
	boost::asio::ip::tcp::resolver resolver(ioService);
	boost::asio::ip::tcp::resolver::query query("localhost", "3000");
	boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);

	boost::asio::connect(sendSoc, it);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	acceptor.accept(clientSocket->socket());
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	clientSocket->start();

	//Data length = 4
	//Object ID = 0
	std::vector<uint8_t> myData1 = {'H', 'C', 'M', 0, 4, 0};
	boost::system::error_code error;
	boost::asio::write(sendSoc, boost::asio::buffer(myData1), boost::asio::transfer_all(), error);

	// Send the data; and let it be received by our socket under test
	ioService.run_one();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ASSERT_EQ((size_t)0, listener.mLastReceivedFrame.size());

	std::vector<uint8_t> myData2 = {'T','E','S','T'};
	boost::asio::write(sendSoc, boost::asio::buffer(myData2), boost::asio::transfer_all(), error);
	// Send the data; and let it be received by our socket under test
	ioService.run_one();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));


	std::vector<uint8_t> expected = {'T','E','S','T'};
	ASSERT_EQ(expected, listener.mLastReceivedFrame);
	ASSERT_EQ(0, listener.mLastObjectId);

	delete clientSocket;
}
