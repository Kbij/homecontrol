/*
 * Client.cpp
 *
 *  Created on: Apr 5, 2016
 *      Author: koen
 */
#include "Comm/Client.h"
#include "Comm/ClientListenerIf.h"
#include "Comm/ClientSocketIf.h"
#include "CommObjects/CommObjectIf.h"
#include <boost/shared_ptr.hpp>
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
		mReceivedObjectString(),
		mAuthenticatedClient()
	{

	};
	virtual ~ClientListenerStub() {};

	CommNs::Client* newClient() {return nullptr;};
	void clientAuthenticated(const CommNs::Client* client, const std::string& name)
	{
		mAuthenticatedClient = name;
	}
	void receiveObject(const std::string name, const CommNs::CommObjectIf* object)
	{
		mReceivedObjectId = object->objectId();
		mReceivedObjectString = object->toString();
	};
	int mReceivedObjectId;
	std::string mReceivedObjectString;
	std::string mAuthenticatedClient;
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
	void unRegisterSocketListener() {};
	void sendFrame(uint8_t objectId, const std::vector<uint8_t>& frame)
	{
		mLastSendObjectId = objectId;
		mLastSendFrame = frame;
	};
	void name(const std::string& name) {};
	boost::asio::io_service mIoService;
	boost::asio::ip::tcp::tcp::socket mSocket;
	uint8_t mLastSendObjectId;
	std::vector<uint8_t> mLastSendFrame;
};
}

TEST(Client, Constructor)
{
	std::shared_ptr<ClientSocketStub> socketStub = std::make_shared<ClientSocketStub>();
	ClientListenerStub* clientListenerStub = new ClientListenerStub;

	// Client becomes owner of socketStub !!
	CommNs::Client* client = new CommNs::Client(socketStub, clientListenerStub);
	delete client;
	delete clientListenerStub;
}

TEST(Client, StartConnection)
{
	std::shared_ptr<ClientSocketStub> socketStub = std::make_shared<ClientSocketStub>();
	ClientListenerStub* clientListenerStub = new ClientListenerStub;
	CommNs::Client* client = new CommNs::Client(socketStub, clientListenerStub);
	client->start();
	//EXPECT_FALSE(client->connected());

	std::string clientNameString = "Client1";
	std::vector<uint8_t> clientNameFrame(clientNameString.begin(), clientNameString.end());
	client->receiveFrame(1, clientNameFrame);

	std::string serverName = "HCM SERVER";
	std::vector<uint8_t> expectedFrame(serverName.begin(), serverName.end());

	// Server should respond with servername
	EXPECT_EQ(2, socketStub->mLastSendObjectId);
	EXPECT_EQ(expectedFrame, socketStub->mLastSendFrame);
	EXPECT_EQ(clientListenerStub->mAuthenticatedClient, "Client1");

	// Should be connected
	//EXPECT_TRUE(client->connected());

	delete client;
	delete clientListenerStub;
}

TEST(Client, ReceiveObject)
{
	std::shared_ptr<ClientSocketStub> socketStub = std::make_shared<ClientSocketStub>();
	ClientListenerStub* clientListenerStub = new ClientListenerStub;
	CommNs::Client* client = new CommNs::Client(socketStub, clientListenerStub);
	client->start();

	//EXPECT_FALSE(client->connected());
	std::string clientNameString = "Client1";

	std::vector<uint8_t> clientNameFrame(clientNameString.begin(), clientNameString.end());
	client->receiveFrame(1, clientNameFrame);

	std::string serverName = "HCM SERVER";
	std::vector<uint8_t> expectedFrame(serverName.begin(), serverName.end());

	// Server should respond with servername
	EXPECT_EQ(2, socketStub->mLastSendObjectId);
	EXPECT_EQ(expectedFrame, socketStub->mLastSendFrame);
	EXPECT_EQ(clientListenerStub->mAuthenticatedClient, "Client1");


	// Should be connected
	//EXPECT_TRUE(client->connected());
	std::string gpsJson(R"({"Accuracy":22,"Latitude":51.0535982,"Longitude":3.6440348,"TimeStamp":"\/Date(1459800687043+0200)\/"})");
	std::vector<uint8_t> gpsFrame(gpsJson.begin(), gpsJson.end());

	client->receiveFrame(10, gpsFrame);

	EXPECT_EQ(10, clientListenerStub->mReceivedObjectId);
	std::cout << "Object: " << clientListenerStub->mReceivedObjectString << std::endl;
	delete client;
	delete clientListenerStub;
}

TEST(Client, TimeOuts)
{
	std::shared_ptr<ClientSocketStub> socketStub = std::make_shared<ClientSocketStub>();
	ClientListenerStub* clientListenerStub = new ClientListenerStub;
	CommNs::Client* client = new CommNs::Client(socketStub, clientListenerStub);
	client->start();

	//EXPECT_FALSE(client->connected());

	//Connecting timeout is 1000 ms
	EXPECT_FALSE(client->isInactive(100)); // Inactive after 100 ms ?
	EXPECT_FALSE(client->isInactive(800)); // Inactive after additional 800 ms ?
	EXPECT_TRUE(client->isInactive(150)); // Inactive after additional 150 ms ?

	std::string clientNameString = "Client1";
	std::vector<uint8_t> clientNameFrame(clientNameString.begin(), clientNameString.end());
	client->receiveFrame(1, clientNameFrame);

	EXPECT_EQ(clientNameString, client->name());
	//EXPECT_TRUE(client->connected());

	//Connected timeout is 5 min
	EXPECT_FALSE(client->isInactive(100)); // Inactive after 100 ms ?
	EXPECT_FALSE(client->isInactive(5*60*1000-200)); // Inactive after additional 5 min (-200 ms) ?

	EXPECT_TRUE(client->isInactive(150)); // Inactive after additional 150 ms

	// Receive an object
	std::string gpsJson(R"({"Accuracy":22,"Latitude":51.0535982,"Longitude":3.6440348,"TimeStamp":"\/Date(1459800687043+0200)\/"})");
	std::vector<uint8_t> gpsFrame(gpsJson.begin(), gpsJson.end());
	client->receiveFrame(10, gpsFrame);

	// Is inactive will go away (in normal case, the server will aready have killed this client)
	EXPECT_FALSE(client->isInactive(100)); // Inactive after additional 150 ms

	delete client;
	delete clientListenerStub;
}
