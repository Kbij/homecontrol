/*
 * JsonFilePidGainStorage.cpp
 *
 *  Created on: Sep 10, 2026
 *      Author: koen
 */

#include "Logic/JsonFilePidGainStorage.h"
#include <json/json.h>
#include <glog/logging.h>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <sys/stat.h>

namespace LogicNs {

JsonFilePidGainStorage::JsonFilePidGainStorage(const std::string& directory):
	mDirectory(directory)
{
	//best effort: create the directory if it doesn't exist yet (e.g. first
	//boot of a freshly imaged device). Not recursive - the parent is
	//expected to already exist. Ignore the result: if it already exists
	//this simply fails harmlessly, and if it genuinely can't be created
	//that will show up (and be logged) on the first save attempt instead.
	::mkdir(mDirectory.c_str(), 0755);
}

JsonFilePidGainStorage::~JsonFilePidGainStorage()
{
}

std::string JsonFilePidGainStorage::filePathFor(const std::string& thermostatId) const
{
	//sanitise the id before using it as a filename: only keep characters
	//that are safe on any filesystem, so a stray '/' or '..' in an id
	//coming from configuration can never escape mDirectory.
	std::string safeId;
	safeId.reserve(thermostatId.size());
	for (char c : thermostatId)
	{
		bool safe = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
				|| c == '-' || c == '_';
		safeId += safe ? c : '_';
	}
	if (safeId.empty()) safeId = "default";

	return mDirectory + "/pid_" + safeId + ".json";
}

bool JsonFilePidGainStorage::loadPidState(const std::string& thermostatId, PidLearnedState& state)
{
	std::string path = filePathFor(thermostatId);
	std::ifstream in(path.c_str());
	if (!in.is_open())
	{
		VLOG(1) << "PidGainStorage: no stored PID state yet at " << path;
		return false;
	}

	std::stringstream buffer;
	buffer << in.rdbuf();

	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(buffer.str(), root, false))
	{
		LOG(WARNING) << "PidGainStorage: failed to parse " << path << ", ignoring stored state";
		return false;
	}

	if (!root.isMember("kp") || !root.isMember("tiSec") || !root.isMember("tdSec")
			|| !root.isMember("referencePeriodSec") || !root.isMember("referenceAmplitude"))
	{
		LOG(WARNING) << "PidGainStorage: " << path << " is missing fields, ignoring stored state";
		return false;
	}

	state.kp = root["kp"].asDouble();
	state.tiSec = root["tiSec"].asDouble();
	state.tdSec = root["tdSec"].asDouble();
	state.referencePeriodSec = root["referencePeriodSec"].asDouble();
	state.referenceAmplitude = root["referenceAmplitude"].asDouble();

	LOG(INFO) << "PidGainStorage: loaded PID state for '" << thermostatId << "' from " << path
			<< " (Kp=" << state.kp << ", Ti=" << state.tiSec << "s, Td=" << state.tdSec << "s)";
	return true;
}

void JsonFilePidGainStorage::savePidState(const std::string& thermostatId, const PidLearnedState& state)
{
	std::string path = filePathFor(thermostatId);
	std::string tmpPath = path + ".tmp";

	Json::Value root;
	root["kp"] = state.kp;
	root["tiSec"] = state.tiSec;
	root["tdSec"] = state.tdSec;
	root["referencePeriodSec"] = state.referencePeriodSec;
	root["referenceAmplitude"] = state.referenceAmplitude;

	Json::FastWriter writer;

	std::ofstream out(tmpPath.c_str(), std::ios::trunc);
	if (!out.is_open())
	{
		LOG(ERROR) << "PidGainStorage: could not open " << tmpPath << " for writing, PID state not saved";
		return;
	}
	out << writer.write(root);
	out.close();

	if (out.fail())
	{
		LOG(ERROR) << "PidGainStorage: failed writing " << tmpPath << ", PID state not saved";
		std::remove(tmpPath.c_str());
		return;
	}

	//atomic on a POSIX filesystem: a crash/power loss either leaves the old
	//file intact or the fully-written new one, never a half-written file.
	if (std::rename(tmpPath.c_str(), path.c_str()) != 0)
	{
		LOG(ERROR) << "PidGainStorage: could not rename " << tmpPath << " to " << path << ", PID state not saved";
		std::remove(tmpPath.c_str());
		return;
	}

	VLOG(1) << "PidGainStorage: saved PID state for '" << thermostatId << "' to " << path;
}

} /* namespace LogicNs */
