/*
 * ServerSocket.h
 *
 *  Created on: Mar 28, 2016
 *      Author: koen
 */

#ifndef COMM_SERVERSOCKET_H_
#define COMM_SERVERSOCKET_H_
#include "ServerSocketIf.h"
#include <boost/asio.hpp>
#include <list>
#include <thread>
#include <mutex>

namespace CommNs
{
class ClientSocketIf;
class Server;
class Client;

class ServerSocket: public ServerSocketIf
{
public:
	ServerSocket(boost::asio::io_service& ioService, Server* server, int port);
	virtual ~ServerSocket();

	void close() {};
private:
	boost::asio::io_service& mIoService;
	Server* mServer;
	boost::asio::ip::tcp::acceptor mAcceptor;
	std::list<ClientSocketIf*> mClients;
	bool mServerThreadRunning;
	std::thread* mServerThread;
	mutable std::recursive_mutex mDataMutex;

	void handleAccept(Client* client, const boost::system::error_code& error);
	void startServerThread();
	void stopServerThread();
	void serverThread();

};

} /* namespace CommNs */

#endif /* COMM_SERVERSOCKET_H_ */
