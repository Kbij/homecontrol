/*
 * LiveTest.cpp
 *
 *  Created on: Apr 5, 2016
 *      Author: koen
 */


#include "Comm/Server.h"
#include "Comm/SocketFactory.h"
#include "Comm/CommListenerIf.h"
#include "Comm/CommObjectIf.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>
#include <iostream>

class CommListenerStub: public CommNs::CommListenerIf
{
public:
	CommListenerStub() {};
	~CommListenerStub() {};
	void receiveObject(const std::string name, const CommNs::CommObjectIf& object)
	{
		std::cout << name << ": " << object.toString() << std::endl;
	};

};

TEST(LiveTest, ServerListening)
{
	CommListenerStub* commListener = new CommListenerStub;
	CommNs::SocketFactory* factory = new CommNs::SocketFactory;
	CommNs::Server* server = new CommNs::Server(factory, 5678);
	server->registerCommListener(commListener);
	std::this_thread::sleep_for(std::chrono::seconds(600));

	delete server;
	delete factory;
	delete commListener;
}
