/*
 * AdminAuthResult.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#include <CommObjects/AdminAuthResult.h>
#include <json/json.h>

namespace CommNs {

AdminAuthResult::AdminAuthResult(bool success):
	mSuccess(success)
{
}

AdminAuthResult::~AdminAuthResult()
{
}

uint8_t AdminAuthResult::objectId() const
{
	return 41;
}

std::string AdminAuthResult::toString() const
{
	return "Admin Auth Result";
}

std::string AdminAuthResult::json() const
{
	Json::Value root;
	root["success"] = mSuccess;

	Json::FastWriter writer;
	return writer.write(root);
}
} /* namespace CommNs */
