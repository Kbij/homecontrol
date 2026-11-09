/*
 * SelfLearningPidThermostat.h
 *
 *  Created on: Sep 10, 2026
 *      Author: koen
 */

#ifndef LOGIC_SELFLEARNINGPIDTHERMOSTAT_H_
#define LOGIC_SELFLEARNINGPIDTHERMOSTAT_H_
#include "ThermostatIf.h"
#include <ctime>
#include <string>

namespace LogicNs {
class ThermostatListenerIf;
class PidGainStorageIf;

// SelfLearningPidThermostat
// --------------------------
// Drop-in replacement for TwoPointThermostat that drives the heater with a
// time-proportioned PID loop instead of simple on/off hysteresis, and that
// tunes its own gains:
//
//  1. On startup it runs a relay-feedback auto-tune (Astrom-Hagglund): the
//     heater is bang-banged on/off around the setpoint until the room
//     temperature settles into a stable oscillation, from which the
//     ultimate gain/period of the real thermal process is estimated and
//     turned into initial Kp/Ti/Td (Tyreus-Luyben rule). This also fixes
//     the output stage's cycle length and minimum on-time (see below) to
//     match how fast the room actually responds.
//  2. Once in normal PID control it keeps adapting Kp (and, more slowly,
//     Ti) online by recognising patterns in the closed-loop error signal -
//     the decay ratio between successive error peaks, and any persistent
//     steady-state offset - the same idea used by classic pattern
//     recognition self-tuners (e.g. Foxboro's EXACT controller). This lets
//     it track slow changes in the room's thermal behaviour (season,
//     insulation, doors/windows, etc.) without ever needing to be
//     re-tuned by hand.
//
// A room is a slow process: it can easily take 5-10 minutes (or more)
// before a heater change shows up as a measurable temperature change. Two
// things in this design specifically account for that:
//  - every timing decision - the auto-tune's oscillation detection, the
//    self-tuner's "is this peak significant/genuine" gating, and the
//    output stage's cycle length - is derived from the room's own
//    identified ultimate period (mReferencePeriodSec), not from a fixed
//    constant. A slow room simply ends up with a longer cycle and a more
//    patient self-tuner; nothing needs to be hand-tuned per installation.
//  - the heater is driven through a fixed-length time-proportioning cycle
//    (see updateDutyOutput()), not by re-deciding on/off from scratch every
//    single 30s sample. Re-deciding every sample is fine mathematically,
//    but it makes a real relay/pump/valve switch far more often than
//    necessary, which is hard on the hardware and pointless when the room
//    can't respond that fast anyway.
//  - because a slow room can also mean a slow auto-tune (its oscillation
//    period is part of what's being measured), the auto-tune's own PID
//    state - once learned - is persisted through an optional
//    PidGainStorageIf, so that a HCServer restart does not force the room
//    through another, possibly many-hours-long, auto-tune.
//
// IMPORTANT DESIGN CONSTRAINT
// ----------------------------
// All computation happens synchronously inside temperatureChanged() and
// nowhere else. This class must never start a thread or a timer of its
// own: temperatureChanged() is the system's heartbeat (it is invoked
// roughly every 30s, whenever a fresh room temperature comes in) and is
// the only place control decisions may be taken. The output stage's cycle
// timing is tracked purely from the time_t timestamps passed into
// temperatureChanged(), never from a real clock/timer of its own.
//
// Because the heater hardware has its own ~60s safety watchdog and shuts
// itself off if it is not re-armed, heaterOn()/heaterOff() is invoked
// unconditionally on *every* temperatureChanged() call - even on ticks
// where the heater's commanded state does not change - so the heater is
// always re-armed comfortably within its timeout as long as temperature
// updates keep arriving at their normal ~30s cadence.
class SelfLearningPidThermostat: public ThermostatIf
{
public:
	// thermostatId:  identifies this thermostat/room, used as the key when
	//                loading/saving learned state through 'storage' (in
	//                practice, the same id RoomControl already uses as its
	//                roomId).
	// storage:       optional persistence for the learned PID state (see
	//                PidGainStorageIf.h / JsonFilePidGainStorage). May be
	//                nullptr to disable persistence entirely, in which
	//                case every restart starts from 'autoTune' below again,
	//                exactly like the non-persisted behaviour. When a
	//                storage is given and it already holds a previously
	//                learned, valid state for thermostatId, that state is
	//                used immediately and no auto-tune is run at all,
	//                however 'autoTune' is set.
	// autoTune:      when no stored state is available (first run, or no
	//                storage given), run the relay-feedback auto-tune
	//                described above on startup to learn initial gains
	//                from the real room response. When false, the
	//                fallback gains below are used directly and no
	//                auto-tune is performed.
	// fallbackKp/TiSec/TdSec: conservative starting gains, used as-is when
	//                autoTune is false, or as a safe fallback if the
	//                auto-tune fails to converge within a bounded time
	//                (see AUTOTUNE_MAX_TICKS in the .cpp).
	SelfLearningPidThermostat(const std::string& thermostatId, PidGainStorageIf* storage = nullptr,
			bool autoTune = true, double fallbackKp = 0.5,
			double fallbackTiSec = 1800.0, double fallbackTdSec = 300.0);
	virtual ~SelfLearningPidThermostat();

	//ThermostatIf
	void registerListener(ThermostatListenerIf* listener);
	void unRegisterListener();
	void temperatureChanged(time_t timePoint, double temperature);
	void setPointChanged(time_t timePoint, double setTemperature);

	//introspection, e.g. for logging/diagnostics
	bool inAutoTune() const { return mMode == Mode::RELAY_AUTOTUNE; }
	double learnedKp() const { return mKp; }
	double learnedTiSec() const { return mTiSec; }
	double learnedTdSec() const { return mTdSec; }
	double outputCycleLengthSec() const { return mCycleLengthSec; }
	bool heaterCommandedOn() const { return mHeaterCommandedOn; }

	// Discards whatever has been learned so far (in memory; the persisted
	// file, if any, is simply overwritten the next time gains are saved)
	// and restarts the relay auto-tune from scratch. Useful after a change
	// that invalidates the old characterisation - a different radiator,
	// insulation work, etc.
	void forgetLearnedState();

private:
	enum class Mode { RELAY_AUTOTUNE, PID_CONTROL };

	void processRelayAutoTune(time_t timePoint, double temperature);
	void processPidControl(time_t timePoint, double temperature, double dt);
	void onRelaySwitch(time_t timePoint, double temperature);
	void finishAutoTune();
	void driveHeater(bool on);
	void updateDutyOutput(double duty, double dt);
	void adaptGains(time_t timePoint, double error, double output);
	void onErrorPeak(time_t timePoint, double peakError);
	void resetPeakTracking();
	void clampGains();
	void deriveOutputTimingFromReference();
	void loadPersistedState();
	void requestSave();
	void saveIfDue(time_t timePoint);
	static double clamp(double value, double lo, double hi);

	const std::string mThermostatId;
	PidGainStorageIf* mStorage;

	ThermostatListenerIf* mListener;
	double mSetPoint;
	bool mHaveSetPoint;

	//sample timing, derived from consecutive temperatureChanged() calls
	time_t mLastTimePoint;
	bool mHaveLastSample;

	Mode mMode;
	bool mStartedFromPersistedState;

	//---- PID gains (Ti/Td in seconds; Kp is the proportional gain on a
	//     [0..1] duty-cycle output) ----
	double mKp;
	double mTiSec;
	double mTdSec;
	double mIntegral;			//running Ki*error*dt accumulator, in duty units
	double mLastTemperature;
	bool mHaveLastTemperature;

	//---- time-proportioning output stage: turns the [0..1] PID duty cycle
	//     into discrete heaterOn()/heaterOff() commands, using a fixed-
	//     length cycle (classic time-proportioning control) instead of
	//     re-deciding on/off from scratch on every single sample - so a
	//     real relay/pump/valve isn't switched far more often than a slow
	//     room could ever need. Cycle length and minimum on-time are
	//     derived from mReferencePeriodSec (see deriveOutputTimingFromReference()),
	//     not hard-coded, so they automatically fit how fast this
	//     particular room responds. All of it is still driven purely by
	//     the timestamps passed into temperatureChanged() - no timer. ----
	double mCycleLengthSec;
	double mMinOnTimeSec;
	double mCycleElapsedSec;
	double mCycleOnTimeTarget;		//how much of the current cycle should be spent ON
	double mOnTimeDebtSec;			//owed on-time that was too small to schedule yet; carried forward so the long-run average duty is still correct
	bool mHeaterCommandedOn;

	//---- relay auto-tune state ----
	bool mRelayOutputOn;
	time_t mLastSwitchTime;
	bool mHaveLastSwitchTime;
	int mHalfCyclesSeen;
	double mCycleMinTemp;
	double mCycleMaxTemp;
	double mLastPeriodSec;
	double mLastAmplitude;
	int mConsistentPeriods;
	int mAutoTuneTicks;

	//---- online gain adaptation (pattern recognition on the closed loop
	//     error signal) ----
	//the process's identified ultimate period (from auto-tune, from
	//persisted state, or a rough estimate derived from the fallback gains
	//when neither is available). Genuine closed-loop instability from too
	//high a gain oscillates near this period; the on/off output stage can
	//only "chatter" at multiples of its cycle length, so peaks spaced far
	//below this period are actuator artifacts, not evidence of poor loop
	//damping, and are ignored. It also sets the output stage's cycle
	//length (see deriveOutputTimingFromReference()).
	double mReferencePeriodSec;
	//the process's identified oscillation amplitude (from auto-tune, from
	//persisted state, or a conservative default otherwise). Only error
	//peaks that are a significant fraction of this size are treated as
	//evidence of a real loop problem; smaller ones are the same order of
	//magnitude as the ripple the on/off output stage produces on its own
	//and are ignored so they can't be mistaken for a tuning issue.
	double mReferenceAmplitude;
	double mLastError;
	int mErrorDirection;			// sign of the last error delta: -1, 0 or +1
	time_t mLastPeakTime;
	bool mHaveLastPeakTime;
	double mLastPeakError;
	bool mHaveLastPeakError;
	double mSettledErrorAvg;
	int mSettledSamples;

	//---- persistence throttling: gains are saved at most once per
	//     mMinSaveIntervalSec, and only if something actually changed,
	//     so a slowly-adapting self-tuner doesn't wear out the flash with
	//     writes that mostly repeat the same values ----
	bool mSavePending;
	time_t mLastSaveTime;
	bool mHaveLastSaveTime;
};

} /* namespace LogicNs */

#endif /* LOGIC_SELFLEARNINGPIDTHERMOSTAT_H_ */
