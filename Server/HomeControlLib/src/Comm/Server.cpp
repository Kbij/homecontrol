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
#include "CommObjects/CommObjectIf.h"
#include "CommObjects/LocationInterval.h"
#include "DAL/HomeControlDalIf.h"
#include <glog/logging.h>


namespace
{
const int MAINTENANCE_INTERVAL_MS = 2000;
}

namespace CommNs {

Server::Server(SocketFactoryIf* factory, int port, DalNs::HomeControlDalIf* dal):
	mDataMutex(),
	mSocketFactory(factory),
	mDal(dal),
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
	if (object)
	{
		std::string json = object->json();
		std::vector<uint8_t> jsonFrame(json.begin(),json.end());
		for(auto client: mClients)
		{
			if (client->name() == name)
			{
				client->sendFrame(object->objectId(), jsonFrame);
			}
		}
		delete object;
	}
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
			VLOG(3) << "Server maintenance, clients: " << mClients.size();

			std::vector<Client*> deletedClients;
			auto clientIt = mClients.begin();
			while (clientIt !=  mClients.end())
			{
				if ((*clientIt)->isInactive(MAINTENANCE_INTERVAL_MS))
				{
					LOG(INFO) << "Deleting client [" << (*clientIt) << "]: " << (*clientIt)->name() << " because it is inactive";
					if ((*clientIt)->name() != "")
					{
						deletedClients.push_back(*clientIt);
					}

					clientIt = mClients.erase(clientIt);

				}
				else
				{
					if (mDal)
					{
						int interval = mDal->locationInterval((*clientIt)->name());
						if (interval != (*clientIt)->locationInterval() && (*clientIt)->name() != "Unknown")
						{
							LOG(INFO) << "Location interval change for client: " << (*clientIt)->name() << ", interval: " << interval;
							(*clientIt)->locationInterval(interval);

							//Sendint the new location to the client
							LocationInterval locationInterval(interval);
							std::string json = locationInterval.json();
							std::vector<uint8_t> jsonFrame(json.begin(),json.end());
							(*clientIt)->sendFrame(locationInterval.objectId(), jsonFrame);
						}
					}

					clientIt++;
				}
			}

			//There could be multiple sockets to the client; check if there is a active socket
			for(auto deletedClient: deletedClients)
			{
				bool clientStillConnected = false;
				for(const auto& client: mClients)
				{
					if (deletedClient->name() == client->name() && deletedClient != client)
					{
						LOG(INFO) << "Client still has an active connection open: " << client->name() << ", not sending clientDisconnected";
						clientStillConnected = true;
					}
				}

				if (!clientStillConnected)
				{
					for(const auto& listener: mCommListeners)
					{
						listener->clientDisConnected(deletedClient->name());
					}
				}

				VLOG(3) << "Deleting client: " << deletedClient->name() << ", addr: " << deletedClient;
				delete deletedClient;
				VLOG(3) << "Client deleted";
			}
		}
	}

	LOG(INFO) << "Maintenance thread stopped";
}
} /* namespace CommNs */
