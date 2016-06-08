/*
 * Server.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#include "Server.h"
#include "Client.h"
#include "ServerSocketIf.h"
#include "SocketFactoryIf.h"
#include "ClientSocketIf.h"
#include "ClientListenerIf.h"
#include "CommListenerIf.h"
#include <glog/logging.h>

namespace
{
const int MAINTENANCE_INTERVAL_MS = 500;
}

namespace CommNs {

Server::Server(SocketFactoryIf* factory, int port):
	mDataMutex(),
	mSocketFactory(factory),
	mServerSocket(nullptr),
	mCommListeners(),
	mClients(),
	mMaintenancehreadRunning(),
	mMaintenanceThread(nullptr)

{
	mServerSocket = factory->createServerSocket(this, port);
	startMaintenanceThread();
}

Server::~Server()
{
	stopMaintenanceThread();
	for(auto client: mClients)
	{
		delete client;
	}
	mClients.clear();
	delete mServerSocket;
}

void Server::registerCommListener(CommListenerIf* listener)
{
	if (listener)
	{
		std::lock_guard<std::mutex> lock(mDataMutex);
		mCommListeners.insert(listener);
	}
}

void Server::unRegisterCommListener(CommListenerIf* listener)
{
	if (listener)
	{
		std::lock_guard<std::mutex> lock(mDataMutex);
		mCommListeners.erase(listener);
	}
}

void Server::sendObject(const std::string name, CommObjectIf* object)
{

}

Client* Server::newClient()
{
	std::lock_guard<std::mutex> lock(mDataMutex);

	Client* client = new Client(mSocketFactory->createClientSocket(), this);
	mClients.push_back(client);
	return client;
}

void Server::clientAuthenticated(const Client* client, const std::string& name)
{
	std::lock_guard<std::mutex> lock(mDataMutex);
	bool wasAlreadyConnected = false;
	for(const auto& serverClient: mClients)
	{
		if (serverClient != client)
		{
			if (serverClient->name() == name)
			{
				wasAlreadyConnected = true;
			}
		}
	}

	if (!wasAlreadyConnected)
	{
		for(const auto& listener: mCommListeners)
		{
			listener->clientConnected(name);
		}
	}
}

void Server::receiveObject(const std::string name, const CommObjectIf* object)
{
	std::lock_guard<std::mutex> lock(mDataMutex);

	for(const auto& listener: mCommListeners)
	{
		listener->receiveObject(name, object);
	}
}

void Server::startMaintenanceThread()
{
	if (!mMaintenancehreadRunning)
	{
		mMaintenancehreadRunning = true;
		mMaintenanceThread = new std::thread(&Server::maintenanceThread, this);
	}
}

void Server::stopMaintenanceThread()
{
	mMaintenancehreadRunning = false;

    if (mMaintenanceThread)
    {
    	mMaintenanceThread->join();
    	delete mMaintenanceThread;
    	mMaintenanceThread = nullptr;
    }
}

void Server::maintenanceThread()
{
	while(mMaintenancehreadRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(MAINTENANCE_INTERVAL_MS));

		{
			std::lock_guard<std::mutex> lock(mDataMutex);
			std::vector<std::string> deletedClients;
			auto clientIt = mClients.begin();
			while (clientIt !=  mClients.end())
			{
				if ((*clientIt)->isInactive(MAINTENANCE_INTERVAL_MS))
				{
					LOG(INFO) << "Deleting client: " << (*clientIt)->name() << " because it is inactive";
					if ((*clientIt)->name() != "")
					{
						deletedClients.push_back((*clientIt)->name());
					}

					clientIt = mClients.erase(clientIt);

				}
				else
				{
					clientIt++;
				}
			}

			for(const auto& deletedClient: deletedClients)
			{
				bool clientStillConnected = false;
				for(const auto& client: mClients)
				{
					if (deletedClient == client->name())
					{
						clientStillConnected = true;
					}
				}

				if (!clientStillConnected)
				{
					for(const auto& listener: mCommListeners)
					{
						listener->clientDisConnected(deletedClient);
					}
				}
			}
		}
	}
}
} /* namespace CommNs */
