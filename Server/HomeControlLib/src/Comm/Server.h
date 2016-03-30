/*
 * Server.h
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#ifndef COMM_SERVER_H_
#define COMM_SERVER_H_
#include "ClientListenerIf.h"

namespace CommNs {
class ServerSocketIf;
class SocketFactoryIf;

class Server: ClientListenerIf
{
public:
	Server(SocketFactoryIf* factory, int port);
	virtual ~Server();

	Client* newClient();
	void receiveObject(const CommObjectIf& object);

private:
	SocketFactoryIf* mSocketFactory;
	ServerSocketIf* mServerSocket;
};

} /* namespace CommNs */

#endif /* COMM_SERVER_H_ */
