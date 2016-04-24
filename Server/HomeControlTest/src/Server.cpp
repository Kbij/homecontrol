/*
 * Server.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "Comm/Server.h"
#include "Comm/Client.h"
#include "Comm/SocketFactory.h"
#include "Comm/CommListenerIf.h"
#include "CommObjects/CommObjectIf.h"
#include "CommObjects/GpsLocation.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>

namespace
{
class CommListenerStub: public CommNs::CommListenerIf
{
public:
	CommListenerStub():
		mLastObjectId(),
		mLastName(),
		mLastConnected(),
		mLastDisConnected()
	{

	}
	~CommListenerStub() {};
	void clientConnected(const std::string& name)
	{
		mLastConnected = name;
	};

	void clientDisConnected(const std::string& name)
	{
		mLastDisConnected = name;
	};

	void receiveObject(const std::string name, const CommNs::CommObjectIf* object)
	{
		mLastObjectId = object->objectId();
		mLastName = name;
	};
	uint8_t mLastObjectId;
	std::string mLastName;
	std::string mLastConnected;
	std::string mLastDisConnected;
};
}
TEST(Server, Constructor)
{
	CommNs::SocketFactory* factory = new CommNs::SocketFactory;
	CommNs::Server* server = new CommNs::Server(factory, 1234);
	CommListenerStub* commListener = new CommListenerStub;

	server->registerCommListener(commListener);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	server->unRegisterCommListener(commListener);

	delete commListener;
	delete server;
	delete factory;
}


TEST(Server, ReceiveFrame)
{
	CommNs::SocketFactory* factory = new CommNs::SocketFactory;
	CommNs::Server* server = new CommNs::Server(factory, 1234);
	CommListenerStub* commListener = new CommListenerStub;
	CommNs::GpsLocation* location = new CommNs::GpsLocation(R"({"Accuracy":22,"Latitude":51.0535982,"Longitude":3.6440348,"TimeStamp":"\/Date(1459800687043+0200)\/"})");
	server->registerCommListener(commListener);
	server->receiveObject("Client", location);

	EXPECT_EQ(10, commListener->mLastObjectId);
	EXPECT_EQ("Client", commListener->mLastName);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	server->unRegisterCommListener(commListener);

	delete location;
	delete commListener;
	delete server;
	delete factory;
}

TEST(Server, ClientConnect)
{
	CommNs::SocketFactory* factory = new CommNs::SocketFactory;
	CommNs::Server* server = new CommNs::Server(factory, 1234);
	CommListenerStub* commListener = new CommListenerStub;
	server->registerCommListener(commListener);
	CommNs::Client* client = server->newClient();

	server->clientAuthenticated(client, "MyClient");
	EXPECT_EQ(commListener->mLastConnected, "MyClient");
	std::this_thread::sleep_for(std::chrono::seconds(1));
	server->unRegisterCommListener(commListener);

	delete commListener;
	delete server;
	delete factory;
}

TEST(Server, ClientDisconnected)
{
	CommNs::SocketFactory* factory = new CommNs::SocketFactory;
	CommNs::Server* server = new CommNs::Server(factory, 1234);
	CommListenerStub* commListener = new CommListenerStub;
	server->registerCommListener(commListener);
	CommNs::Client* client = server->newClient();

	// There will be a socket error
	client->start();
	std::vector<uint8_t> frame({'C', 'l', 'i', 'e', 'n', 't'});

	//Client will be "connected" after this frame; but due to the socket error, Client::socketClosed() will be called
	client->receiveFrame(0, frame);

	EXPECT_EQ(commListener->mLastConnected, "Client");

	std::this_thread::sleep_for(std::chrono::seconds(1));

	// Because the socketClosed() call, the client will be immediately closed
	EXPECT_EQ(commListener->mLastDisConnected, "Client");

	server->unRegisterCommListener(commListener);

	delete commListener;
	delete server;
	delete factory;
}

