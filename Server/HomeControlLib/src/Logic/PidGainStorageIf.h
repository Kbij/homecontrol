/*
 * PidGainStorageIf.h
 *
 *  Created on: Sep 10, 2026
 *      Author: koen
 */

#ifndef LOGIC_PIDGAINSTORAGEIF_H_
#define LOGIC_PIDGAINSTORAGEIF_H_
#include <string>

namespace LogicNs {

// What SelfLearningPidThermostat learns about a room's thermal behaviour,
// and persists so it survives a HCServer restart:
//  - kp/tiSec/tdSec: the current PID gains (from auto-tune and/or the
//    online self-tuner).
//  - referencePeriodSec/referenceAmplitude: the room's identified thermal
//    "signature" from the relay auto-tune (the ultimate period and
//    oscillation amplitude). These are what let the self-tuner tell a
//    genuine control-loop oscillation apart from ordinary actuator ripple,
//    and what the output stage's cycle length is derived from - without
//    them a restarted process would have to run a fresh (and, for a slow
//    room, potentially many-hours-long) auto-tune just to get them back.
struct PidLearnedState
{
	double kp;
	double tiSec;
	double tdSec;
	double referencePeriodSec;
	double referenceAmplitude;
};

// PidGainStorageIf
// -----------------
// Persists the learned PID state above across restarts, so a room's
// thermostat doesn't have to repeat its (possibly hours-long, for a slow
// room) relay auto-tune every time the HCServer process restarts.
//
// A single HCServer instance controls multiple rooms (see RoomControl),
// each with its own SelfLearningPidThermostat, so every call is keyed by
// thermostatId - in practice the same id RoomControl uses as its roomId.
class PidGainStorageIf
{
public:
	virtual ~PidGainStorageIf() {};

	// Returns true and fills 'state' if a previously learned state exists
	// for this thermostat; returns false (and leaves 'state' untouched) if
	// nothing has been learned/stored yet, e.g. on first-ever startup.
	virtual bool loadPidState(const std::string& thermostatId, PidLearnedState& state) = 0;

	// Persists (overwrites) the learned state for this thermostat. Called
	// only occasionally - see SelfLearningPidThermostat's save throttling -
	// so implementations do not need to worry about being called every
	// temperatureChanged() tick.
	virtual void savePidState(const std::string& thermostatId, const PidLearnedState& state) = 0;
};

} /* namespace LogicNs */

#endif /* LOGIC_PIDGAINSTORAGEIF_H_ */
