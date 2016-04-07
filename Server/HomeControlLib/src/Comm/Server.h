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
#include <list>
#include <thread>

namespace CommNs {
class ServerSocketIf;
class SocketFactoryIf;
class CommListenerIf;
class Client;

class Server: ClientListenerIf
{
public:
	Server(SocketFactoryIf* factory, int port);
	virtual ~Server();

	void registerCommListener(CommListenerIf* listener);
	void unRegisterCommListener(CommListenerIf* listener);
	Client* newClient();
	void receiveObject(const std::string name, const CommObjectIf& object);

private:
	mutable std::mutex mDataMutex;
	SocketFactoryIf* mSocketFactory;
	ServerSocketIf* mServerSocket;
	std::set<CommListenerIf*> mCommListeners;
	std::list<Client*> mClients;
	bool mMaintenancehreadRunning;
	std::thread* mMaintenanceThread;


	void startMaintenanceThread();
	void stopMaintenanceThread();
	void maintenanceThread();


};

} /* namespace CommNs */

#endif /* COMM_SERVER_H_ */
