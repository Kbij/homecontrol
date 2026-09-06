/*
 * AdminCapability.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#include <CommObjects/AdminCapability.h>
#include <json/json.h>

namespace CommNs {

AdminCapability::AdminCapability(bool isAdmin):
	mIsAdmin(isAdmin)
{
}

AdminCapability::~AdminCapability()
{
}

uint8_t AdminCapability::objectId() const
{
	return 39;
}

std::string AdminCapability::toString() const
{
	return "Admin Capability";
}

std::string AdminCapability::json() const
{
	Json::Value root;
	root["isAdmin"] = mIsAdmin;

	Json::FastWriter writer;
	return writer.write(root);
}
} /* namespace CommNs */
