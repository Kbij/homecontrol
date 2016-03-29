/*
 * ServerSocket.h
 *
 *  Created on: Mar 28, 2016
 *      Author: koen
 */

#ifndef COMM_SERVERSOCKET_H_
#define COMM_SERVERSOCKET_H_

#include <boost/asio.hpp>
#include <list>
#include <thread>
#include <mutex>

namespace CommNs
{
class ClientSocketIf;

class ServerSocket {
public:
	ServerSocket(int port);
	virtual ~ServerSocket();
	void handleAccept(ClientSocketIf* client, const boost::system::error_code& error);
private:
	boost::asio::io_service mIoService;
	boost::asio::ip::tcp::acceptor mAcceptor;
	std::list<ClientSocketIf*> mClients;
	bool mServerThreadRunning;
	std::thread* mServerThread;
	mutable std::recursive_mutex mDataMutex;

	void startServerThread();
	void stopServerThread();
	void serverThread();

};

} /* namespace CommNs */

#endif /* COMM_SERVERSOCKET_H_ */
