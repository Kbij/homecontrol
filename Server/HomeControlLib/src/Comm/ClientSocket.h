/*
 * Client.h
 *
 *  Created on: Mar 29, 2016
 *      Author: koen
 */

#ifndef COMM_CLIENTSOCKET_H_
#define COMM_CLIENTSOCKET_H_
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/system/system_error.hpp>
#include <cstddef>
#include <string>
#include "ClientSocketIf.h"

namespace CommNs {

class ClientSocket: public ClientSocketIf
{
public:
	ClientSocket(boost::asio::io_service& ioService);
	virtual ~ClientSocket();

	boost::asio::ip::tcp::tcp::socket& socket();
	void start();
	std::string name() {return "unknown";}
	void close();

private:
	boost::asio::ip::tcp::tcp::socket mSocket;
	boost::array<char, 1024> mBuffer;
	void handleRead(const boost::system::error_code& error, size_t bytesTransferred);
};

} /* namespace CommNs */

#endif /* COMM_CLIENTSOCKET_H_ */
