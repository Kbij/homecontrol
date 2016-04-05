/*
 * Client.h
 *
 *  Created on: Mar 29, 2016
 *      Author: koen
 */

#ifndef COMM_CLIENTSOCKET_H_
#define COMM_CLIENTSOCKET_H_
#include "ClientSocketIf.h"
#include "SocketListenerIf.h"
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/system/system_error.hpp>
#include <cstddef>
#include <string>
#include <vector>
#include <stdint.h>

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
	void registerSocketListener(SocketListenerIf* socketListener);
	void sendFrame(uint8_t objectId, const std::vector<uint8_t>& frame);

private:
	boost::asio::ip::tcp::tcp::socket mSocket;
	boost::array<char, 1024> mSocketBuffer;
	std::vector<uint8_t> mReceiveBuffer;
	SocketListenerIf* mSocketListener;
	void handleRead(const boost::system::error_code& error, size_t bytesTransferred);
	void processBuffer();
};

} /* namespace CommNs */

#endif /* COMM_CLIENTSOCKET_H_ */
