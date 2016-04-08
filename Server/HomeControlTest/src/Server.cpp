/*
 * Server.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "Comm/Server.h"
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
		mLastName()
	{

	}
	~CommListenerStub() {};
	void receiveObject(const std::string name, const CommNs::CommObjectIf* object)
	{
		mLastObjectId = object->objectId();
		mLastName = name;
	};
	uint8_t mLastObjectId;
	std::string mLastName;
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
