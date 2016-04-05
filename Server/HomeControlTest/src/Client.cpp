/*
 * Client.cpp
 *
 *  Created on: Apr 5, 2016
 *      Author: koen
 */
#include "Comm/Client.h"
#include "Comm/ClientListenerIf.h"
#include "Comm/ClientSocketIf.h"
#include "Comm/CommObjectIf.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>
#include <vector>
#include <stdint.h>
#include <boost/asio.hpp>
#include <iostream>

namespace
{

class ClientListenerStub: public CommNs::ClientListenerIf
{
public:
	ClientListenerStub():
		mReceivedObjectId(),
		mReceivedObjectString()
	{

	};
	virtual ~ClientListenerStub() {};

	CommNs::Client* newClient() {return nullptr;};
	void receiveObject(const std::string name, const CommNs::CommObjectIf& object)
	{
		mReceivedObjectId = object.objectId();
		mReceivedObjectString = object.toString();
	};
	int mReceivedObjectId;
	std::string mReceivedObjectString;
};

class ClientSocketStub: public CommNs::ClientSocketIf
{
public:
	ClientSocketStub():
		mIoService(),
		mSocket(mIoService),
		mLastSendObjectId(),
		mLastSendFrame()

	{
	};
	virtual ~ClientSocketStub() {};

	boost::asio::ip::tcp::tcp::socket& socket() {return mSocket;};
	void start() {};
	std::string name() {return "Test";};
	void close() {};
	void registerSocketListener(CommNs::SocketListenerIf* socketListener) {};
	void sendFrame(uint8_t objectId, const std::vector<uint8_t>& frame)
	{
		mLastSendObjectId = objectId;
		mLastSendFrame = frame;
	};
	boost::asio::io_service mIoService;
	boost::asio::ip::tcp::tcp::socket mSocket;
	uint8_t mLastSendObjectId;
	std::vector<uint8_t> mLastSendFrame;
};
}

TEST(Client, Constructor)
{
	ClientSocketStub* socketStub = new ClientSocketStub;
	ClientListenerStub* clientListenerStub = new ClientListenerStub;
	CommNs::Client* client = new CommNs::Client(socketStub, clientListenerStub);
	delete client;
	delete clientListenerStub;
	delete socketStub;
}

TEST(Client, StartConnection)
{
	ClientSocketStub* socketStub = new ClientSocketStub;
	ClientListenerStub* clientListenerStub = new ClientListenerStub;
	CommNs::Client* client = new CommNs::Client(socketStub, clientListenerStub);
	//EXPECT_FALSE(client->connected());
	std::string clientNameString = "Client1";

	std::vector<uint8_t> clientNameFrame(clientNameString.begin(), clientNameString.end());
	client->receiveFrame(1, clientNameFrame);

	std::string serverName = "HCM SERVER";
	std::vector<uint8_t> expectedFrame(serverName.begin(), serverName.end());

	// Server should respond with servername
	EXPECT_EQ(2, socketStub->mLastSendObjectId);
	EXPECT_EQ(expectedFrame, socketStub->mLastSendFrame);

	// Should be connected
	EXPECT_TRUE(client->connected());

	delete client;
	delete clientListenerStub;
	delete socketStub;
}

TEST(Client, ReceiveObject)
{
	ClientSocketStub* socketStub = new ClientSocketStub;
	ClientListenerStub* clientListenerStub = new ClientListenerStub;
	CommNs::Client* client = new CommNs::Client(socketStub, clientListenerStub);
	//EXPECT_FALSE(client->connected());
	std::string clientNameString = "Client1";

	std::vector<uint8_t> clientNameFrame(clientNameString.begin(), clientNameString.end());
	client->receiveFrame(1, clientNameFrame);

	std::string serverName = "HCM SERVER";
	std::vector<uint8_t> expectedFrame(serverName.begin(), serverName.end());

	// Server should respond with servername
	EXPECT_EQ(2, socketStub->mLastSendObjectId);
	EXPECT_EQ(expectedFrame, socketStub->mLastSendFrame);

	// Should be connected
	EXPECT_TRUE(client->connected());
	std::string gpsJson(R"({"Accuracy":22,"Latitude":51.0535982,"Longitude":3.6440348,"TimeStamp":"\/Date(1459800687043+0200)\/"})");
	std::vector<uint8_t> gpsFrame(gpsJson.begin(), gpsJson.end());

	client->receiveFrame(10, gpsFrame);

	EXPECT_EQ(10, clientListenerStub->mReceivedObjectId);
	std::cout << "Object: " << clientListenerStub->mReceivedObjectString << std::endl;
	delete client;
	delete clientListenerStub;
	delete socketStub;
}
