/*
 * TemperatureMonitoring.cpp
 *
 *  Created on: Jun 12, 2016
 *      Author: koen
 */

#include <CommObjects/TemperatureMonitoring.h>
#include "json/json.h"

namespace CommNs {

TemperatureMonitoring::TemperatureMonitoring():
	mMonitoringEnabled(false)
{
}

TemperatureMonitoring::TemperatureMonitoring(const std::string& json):
	mMonitoringEnabled(false)
{
	Json::Value jsonRoot;
    Json::Reader reader;
    if (reader.parse(json, jsonRoot, false))
    {
    	if (jsonRoot.isMember("monitoringEnabled"))
    	{
    		mMonitoringEnabled = jsonRoot["monitoringEnabled"].asBool();
    	}
    }
}

TemperatureMonitoring::~TemperatureMonitoring()
{
}

uint8_t TemperatureMonitoring::objectId() const
{
	return 20;
}

std::string TemperatureMonitoring::toString() const
{
	return "";
}

std::string TemperatureMonitoring::json() const
{
	return "";
}
} /* namespace CommNs */
