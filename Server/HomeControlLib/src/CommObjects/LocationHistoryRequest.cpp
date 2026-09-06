/*
 * LocationHistoryRequest.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#include <CommObjects/LocationHistoryRequest.h>
#include <json/json.h>

namespace CommNs {

LocationHistoryRequest::LocationHistoryRequest(const std::string& json):
	mClientName(),
	mMinutes(0)
{
	Json::Value jsonRoot;
	Json::Reader reader;
	if (reader.parse(json, jsonRoot, false))
	{
		if (jsonRoot.isMember("client"))
		{
			mClientName = jsonRoot["client"].asString();
		}
		if (jsonRoot.isMember("minutes"))
		{
			mMinutes = jsonRoot["minutes"].asInt();
		}
	}
}

LocationHistoryRequest::~LocationHistoryRequest()
{
}

uint8_t LocationHistoryRequest::objectId() const
{
	return 43;
}

std::string LocationHistoryRequest::toString() const
{
	return "Location History Request, client: " + mClientName;
}
} /* namespace CommNs */
