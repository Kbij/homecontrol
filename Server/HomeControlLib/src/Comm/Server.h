/*
 * Server.h
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#ifndef COMM_SERVER_H_
#define COMM_SERVER_H_

#include "ClientListenerIf.h"
#include <set>
#include <mutex>

namespace CommNs {
class ServerSocketIf;
class SocketFactoryIf;
class CommListenerIf;

class Server: ClientListenerIf
{
public:
	Server(SocketFactoryIf* factory, int port);
	virtual ~Server();

	void registerCommListener(CommListenerIf* listener);
	void unRegisterCommListener(CommListenerIf* listener);
	Client* newClient();
	void receiveObject(const CommObjectIf& object);

private:
	SocketFactoryIf* mSocketFactory;
	ServerSocketIf* mServerSocket;
	std::set<CommListenerIf*> mCommListeners;
	mutable std::mutex mDataMutex;

};

} /* namespace CommNs */

#endif /* COMM_SERVER_H_ */
