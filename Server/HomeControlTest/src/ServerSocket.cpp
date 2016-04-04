/*
 * ServerSocket.cpp
 *
 *  Created on: Mar 28, 2016
 *      Author: koen
 */



#include "Comm/ServerSocket.h"
#include "Comm/SocketFactory.h"
#include "Comm/Server.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <string>
#include <thread>
//
//class SocketFactory: public CommNs::SocketFactoryIf
//{
//public:
//	virtual ~SocketFactoryIf() {};
//
//	ServerSocketIf* createServerSocket(CommNs::Server* server, int port)
//	{
//
//	}
//
//	ClientSocketIf* createClientSocket()
//	{
//
//	}
//
//};
//
//
//TEST(ServerSocket, Constructor)
//{
//	CommNs::Server* server = new CommNs::Server
//	CommNs::ServerSocket* socket = new CommNs::ServerSocket(1234);
////
////	std::this_thread::sleep_for(std::chrono::seconds(10));
////
////	delete socket;
//}
