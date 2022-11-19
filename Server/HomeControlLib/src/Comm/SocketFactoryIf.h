/*
 * SocketFactoryIf.h
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#ifndef COMM_SOCKETFACTORYIF_H_
#define COMM_SOCKETFACTORYIF_H_
#include <boost/asio.hpp>
#include <memory>

namespace CommNs {
class ServerSocketIf;
class ClientSocketIf;
class Server;

class SocketFactoryIf
{
public:
	virtual ~SocketFactoryIf() {};

	virtual ServerSocketIf* createServerSocket(Server* server, int port) = 0;
	virtual std::shared_ptr<ClientSocketIf> createClientSocket() = 0;
};

} /* namespace CommNs */


#endif /* COMM_SOCKETFACTORYIF_H_ */
