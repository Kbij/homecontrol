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
#include <boost/enable_shared_from_this.hpp>
#include <cstddef>
#include <string>
#include <vector>
#include <stdint.h>

namespace CommNs {

//Need shared_from_this: closing the socket does not imediately cancel all outstanding async calls.
//This causes problems while destructing the object: you close the socket, and some async calls remain open
class ClientSocket: public ClientSocketIf, public boost::enable_shared_from_this<ClientSocket>
{
public:
	ClientSocket(boost::asio::io_service& ioService);
	virtual ~ClientSocket();

	boost::asio::ip::tcp::tcp::socket& socket();
	void start();
	std::string name() {return mName;}

	void registerSocketListener(SocketListenerIf* socketListener);
	void unRegisterSocketListener();
	void sendFrame(uint8_t objectId, const std::vector<uint8_t>& frame);
	void name(const std::string& name) {mName = name;};
private:
	boost::asio::ip::tcp::tcp::socket mSocket;
	boost::array<char, 1024> mSocketBuffer;
	std::vector<uint8_t> mReceiveBuffer;
	std::vector<uint8_t> mSendBuffer;
	SocketListenerIf* mSocketListener;
	std::string mName;
	unsigned short mLocalPort;
	void handleRead(const boost::system::error_code& error, size_t bytesTransferred);
	void handleWrite(const boost::system::error_code& error, std::size_t bytesTransferred);
	void processBuffer();
};

} /* namespace CommNs */

#endif /* COMM_CLIENTSOCKET_H_ */
