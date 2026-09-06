/*
 * AdminAuthRequest.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#include <CommObjects/AdminAuthRequest.h>
#include <json/json.h>

namespace CommNs {

AdminAuthRequest::AdminAuthRequest(const std::string& json):
	mCode()
{
	Json::Value jsonRoot;
	Json::Reader reader;
	if (reader.parse(json, jsonRoot, false))
	{
		if (jsonRoot.isMember("code"))
		{
			mCode = jsonRoot["code"].asString();
		}
	}
}

AdminAuthRequest::~AdminAuthRequest()
{
}

uint8_t AdminAuthRequest::objectId() const
{
	return 40;
}

std::string AdminAuthRequest::toString() const
{
	return "Admin Auth Request";
}
} /* namespace CommNs */
