/*
 * Server.cpp
 *
 *  Created on: Mar 30, 2016
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

class CommListenerStub: public CommNs::CommListenerIf
{
public:
	CommListenerStub():
		mLastObjectId()
	{

	}
	~CommListenerStub() {};
	void receiveObject(const CommNs::CommObjectIf& object) {mLastObjectId = object.objectId();};
	uint8_t mLastObjectId;
};

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

	server->registerCommListener(commListener);

	boost::asio::io_service ioService;
	boost::asio::ip::tcp::socket sendSoc(ioService);
	boost::asio::ip::tcp::resolver resolver(ioService);
	boost::asio::ip::tcp::resolver::query query("localhost", "1234");
	boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
	boost::asio::connect(sendSoc, it);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ioService.run_one();

	std::string gpsJson(R"({"Accuracy":22,"Latitude":51.0535982,"Longitude":3.6440348,"TimeStamp":"\/Date(1459800687043+0200)\/"})");
	std::string greeting("HCM");
	std::vector<uint8_t> data(greeting.begin(), greeting.end());
	//Length
	data.push_back(0);
	data.push_back(gpsJson.size());

	//ObjectId
	data.push_back(10);
	data.insert(data.end(), gpsJson.begin(), gpsJson.end());
	boost::system::error_code error;
	boost::asio::write(sendSoc, boost::asio::buffer(data), boost::asio::transfer_all(), error);
	ioService.run_one();
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	EXPECT_EQ(commListener->mLastObjectId, 10);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	server->unRegisterCommListener(commListener);
	delete server;
	delete factory;
}
