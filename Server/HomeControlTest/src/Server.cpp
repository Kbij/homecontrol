/*
 * Server.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "Comm/Server.h"
#include "Comm/SocketFactory.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>

TEST(Server, Constructor)
{
	CommNs::SocketFactory* factory = new CommNs::SocketFactory;
	CommNs::Server* server = new CommNs::Server(factory, 1234);

	std::this_thread::sleep_for(std::chrono::seconds(10));

	delete server;
	delete factory;
}

