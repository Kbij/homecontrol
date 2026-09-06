/*
 * ClientList.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#include <CommObjects/ClientList.h>
#include <json/json.h>

namespace CommNs {

ClientList::ClientList()
{
}

ClientList::~ClientList()
{
}

uint8_t ClientList::objectId() const
{
	return 42;
}

std::string ClientList::toString() const
{
	return "Client List";
}

std::string ClientList::json() const
{
	Json::Value root;
	Json::Value clientArray;
	for(const auto& name: mClients)
	{
		clientArray.append(name);
	}

	root["clients"] = clientArray;

	Json::FastWriter writer;
	return writer.write(root);
}

void ClientList::addClient(const std::string& name)
{
	mClients.push_back(name);
}
} /* namespace CommNs */
