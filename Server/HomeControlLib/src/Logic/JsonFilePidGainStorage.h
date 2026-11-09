/*
 * JsonFilePidGainStorage.h
 *
 *  Created on: Sep 10, 2026
 *      Author: koen
 */

#ifndef LOGIC_JSONFILEPIDGAINSTORAGE_H_
#define LOGIC_JSONFILEPIDGAINSTORAGE_H_
#include "PidGainStorageIf.h"

namespace LogicNs {

// JsonFilePidGainStorage
// ------------------------
// Simple PidGainStorageIf implementation that keeps one small JSON file per
// thermostat on the local filesystem - no database round trip needed just
// to remember a handful of doubles. Meant for the embedded target this
// runs on: writes go through a temp-file-then-rename so a power loss mid
// write can never corrupt the file (rename() is atomic), and the caller
// controls how often savePidState() is actually invoked (see
// SelfLearningPidThermostat's save throttling), so flash wear from
// frequent small writes is not a concern in practice.
//
// 'directory' should point at a writable, persistent (i.e. not tmpfs)
// location - e.g. wherever this device already keeps its other local
// state. The directory itself is created (a single mkdir, not recursive)
// if it does not exist yet; its parent must already exist.
class JsonFilePidGainStorage: public PidGainStorageIf
{
public:
	explicit JsonFilePidGainStorage(const std::string& directory);
	virtual ~JsonFilePidGainStorage();

	//PidGainStorageIf
	bool loadPidState(const std::string& thermostatId, PidLearnedState& state);
	void savePidState(const std::string& thermostatId, const PidLearnedState& state);

private:
	std::string filePathFor(const std::string& thermostatId) const;

	const std::string mDirectory;
};

} /* namespace LogicNs */

#endif /* LOGIC_JSONFILEPIDGAINSTORAGE_H_ */
