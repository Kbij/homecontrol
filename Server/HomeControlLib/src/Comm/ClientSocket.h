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
#include <mutex>
#include <memory>

namespace CommNs
{
struct ReceivedFrame
{
	ReceivedFrame(): ObjectId(), Frame() {};
	uint8_t ObjectId;
	std::vector<uint8_t> Frame;
};

class ClientSocket: public ClientSocketIf, public std::enable_shared_from_this<ClientSocket>
{
public:
	ClientSocket(boost::asio::io_service& ioService);
	virtual ~ClientSocket();

	boost::asio::ip::tcp::tcp::socket& socket();
	void start();
	void close();
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
	std::mutex mDataMutex;
	void handleRead(const boost::system::error_code& error, size_t bytesTransferred);
	void handleWrite(const boost::system::error_code& error, std::size_t bytesTransferred);
	std::vector<ReceivedFrame> processBuffer();
};

} /* namespace CommNs */

#endif /* COMM_CLIENTSOCKET_H_ */
