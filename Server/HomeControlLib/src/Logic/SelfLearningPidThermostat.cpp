/*
 * SelfLearningPidThermostat.cpp
 *
 *  Created on: Sep 10, 2026
 *      Author: koen
 */

#include "Logic/SelfLearningPidThermostat.h"
#include "Logic/ThermostatListenerIf.h"
#include "Logic/PidGainStorageIf.h"
#include <glog/logging.h>
#include <cmath>
#include <algorithm>

namespace
{
const double PI = 3.14159265358979323846;

//nominal spacing between temperatureChanged() calls. Used as a fallback dt
//and as the floor for the derived minimum on-time below.
const double NOMINAL_SAMPLE_INTERVAL_SEC = 30.0;

//--- relay auto-tune ---
//half of this band on either side of the setpoint is where the relay
//switches; must be comfortably above sensor noise/quantisation. Note this
//is the same order of magnitude as a classic two-point thermostat's
//hysteresis, so comfort during auto-tune is no worse than TwoPointThermostat
//was - the room is simply being controlled the "old" way while its
//response is being measured.
const double RELAY_HYSTERESIS = 0.3;
//ignore the first two relay half-cycles: the loop hasn't settled into a
//real oscillation yet and this reading would skew the identification.
const int AUTOTUNE_SKIP_HALF_CYCLES = 2;
//require this many consecutive periods to agree before trusting them.
const int AUTOTUNE_REQUIRED_CONSISTENT_PERIODS = 2;
//two periods are considered "consistent" if they are within 20% of
//each other.
const double AUTOTUNE_PERIOD_TOLERANCE = 0.2;
//amplitude readings below this are assumed to be sensor noise, not a real
//oscillation.
const double AUTOTUNE_MIN_USEFUL_AMPLITUDE = 0.05;
//safety cap: if the relay test hasn't converged after this many ticks, give
//up and fall back to the configured default gains rather than measuring
//forever. A room can easily need 30-90+ minutes per half-cycle once its
//transport delay and thermal mass are accounted for, and 3 periods are
//needed to trust the result, so this is deliberately generous (~36 hours)
//rather than tuned for a fast lab process. Two things make that acceptable
//in practice: comfort during auto-tune is no worse than plain two-point
//control (see RELAY_HYSTERESIS above), and - with a PidGainStorageIf
//configured - this is a one-off cost per room, not something that repeats
//on every restart.
const int AUTOTUNE_MAX_TICKS = 4320;

//--- PID gain bounds, to keep the self-tuner from ever wandering into an
//    unstable region ---
const double KP_MIN = 0.02;
const double KP_MAX = 5.0;
const double TI_MIN_SEC = 60.0;
const double TI_MAX_SEC = 21600.0;	//6h: a slow room can genuinely need a long integral time
const double TD_MIN_SEC = 0.0;
const double TD_MAX_SEC = 3600.0;

//--- online gain adaptation (pattern recognition on the closed-loop error) ---
//deadband on the error delta, to keep sensor jitter from being mistaken
//for a direction change.
const double PEAK_NOISE_MARGIN = 0.02;
//an error peak only counts as evidence of a real closed-loop problem if
//it is at least this fraction of the auto-tune's identified oscillation
//amplitude; smaller peaks are the kind of ripple the on/off output stage
//produces by itself and say nothing about the tuning.
const double SIGNIFICANT_PEAK_FRACTION_OF_REFERENCE_AMPLITUDE = 0.5;
//absolute floor under the above, so a tiny/zero reference amplitude (e.g.
//auto-tune disabled) can't make the peak detector oversensitive to noise.
const double MIN_PEAK_MAGNITUDE_FLOOR = 0.1;
//reference amplitude assumed when auto-tune is disabled or inconclusive:
//deliberately conservative (large), since without a real relay-test
//measurement there is no basis to react to small peaks.
const double DEFAULT_REFERENCE_AMPLITUDE = 1.0;
//classic Ziegler-Nichols "quarter amplitude damping" targets a decay ratio
//of about 0.25 between successive error peaks. Outside this band we nudge
//Kp back towards it.
const double OSCILLATION_DECAY_RATIO_HIGH = 0.5;	//poorly damped -> reduce gain
const double OSCILLATION_DECAY_RATIO_LOW = 0.1;	//overdamped   -> raise gain a little
const double GAIN_ADAPT_DOWN_FACTOR = 0.9;
const double GAIN_ADAPT_UP_FACTOR = 1.05;
//a pair of error peaks is only treated as evidence of a genuine, poorly
//damped control-loop oscillation if they are at least this fraction of the
//process's ultimate period apart; anything faster is assumed to be
//quantisation ripple from the on/off output stage, not a tuning problem.
const double MIN_OSCILLATION_PERIOD_FRACTION_OF_TU = 0.25;
//...and also at least this many output cycles apart. mReferencePeriodSec
//can, for a fast process, end up close to the output stage's own cycle
//length once MIN_CYCLE_LENGTH_SEC clamps it, which would otherwise let the
//output stage's own on/off cadence slip past the Tu-based check above and
//be mistaken for a real oscillation. Requiring several full cycles between
//peaks closes that gap regardless of how the two happen to compare.
const double MIN_OSCILLATION_PERIOD_CYCLES = 3.0;

//persistent steady-state offset detection (adapts Ti)
const double SETTLED_ERROR_BAND = 0.5;
const double BIAS_TRIGGER = 0.15;
const double BIAS_FILTER_ALPHA = 0.02;
const int BIAS_MIN_SAMPLES = 40;
const double TI_ADAPT_DOWN_FACTOR = 0.85;

//a setpoint step bigger than this is treated as a deliberate new target
//rather than the small nudges from the room's up/down buttons, and resets
//whichever measurement (auto-tune or peak tracking) is in progress.
const double SETPOINT_JUMP_THRESHOLD = 0.5;

//--- time-proportioning output stage ---
//the cycle length is derived from the room's identified ultimate period
//(mReferencePeriodSec), not hard-coded: a fast process gets a short cycle,
//a slow one (a typical room easily needs 5-10+ minutes to show any
//measurable response at all) gets a longer one automatically, so a real
//relay/pump/valve is never switched far more often than the room could
//ever respond to.
const double CYCLE_LENGTH_TU_FRACTION = 10.0;
const double MIN_CYCLE_LENGTH_SEC = 180.0;		//3 minutes
const double MAX_CYCLE_LENGTH_SEC = 1800.0;	//30 minutes
//minimum meaningful on-pulse within a cycle, also derived from the cycle
//length. A requested pulse shorter than this is not delivered at all -
//its owed on-time is carried forward (mOnTimeDebtSec) to a later cycle
//instead - rather than switching a real heater/pump/valve on for just a
//few seconds.
const double MIN_ON_TIME_FRACTION_OF_CYCLE = 0.1;
const double MIN_ON_TIME_FLOOR_SEC = NOMINAL_SAMPLE_INTERVAL_SEC;
const double MIN_ON_TIME_CEIL_SEC = 300.0;		//5 minutes

//--- persisted-state save throttling ---
//gains are saved at most this often, so a slowly-adapting self-tuner does
//not wear out the (typically flash) storage with frequent small writes.
const double MIN_SAVE_INTERVAL_SEC = 900.0;	//15 minutes
}

namespace LogicNs {

SelfLearningPidThermostat::SelfLearningPidThermostat(const std::string& thermostatId, PidGainStorageIf* storage,
		bool autoTune, double fallbackKp, double fallbackTiSec, double fallbackTdSec):
	mThermostatId(thermostatId),
	mStorage(storage),
	mListener(nullptr),
	mSetPoint(16.0),
	mHaveSetPoint(false),
	mLastTimePoint(0),
	mHaveLastSample(false),
	mMode(autoTune ? Mode::RELAY_AUTOTUNE : Mode::PID_CONTROL),
	mStartedFromPersistedState(false),
	mKp(fallbackKp),
	mTiSec(fallbackTiSec),
	mTdSec(fallbackTdSec),
	mIntegral(0.0),
	mLastTemperature(0.0),
	mHaveLastTemperature(false),
	mCycleLengthSec(MIN_CYCLE_LENGTH_SEC),
	mMinOnTimeSec(NOMINAL_SAMPLE_INTERVAL_SEC),
	mCycleElapsedSec(0.0),
	mCycleOnTimeTarget(0.0),
	mOnTimeDebtSec(0.0),
	mHeaterCommandedOn(false),
	mRelayOutputOn(false),
	mLastSwitchTime(0),
	mHaveLastSwitchTime(false),
	mHalfCyclesSeen(0),
	mCycleMinTemp(0.0),
	mCycleMaxTemp(0.0),
	mLastPeriodSec(0.0),
	mLastAmplitude(0.0),
	mConsistentPeriods(0),
	mAutoTuneTicks(0),
	//when auto-tune is disabled (and nothing is loaded from storage below)
	//there is no measured Tu yet; approximate it from the fallback gains
	//using the same Tyreus-Luyben relation used to derive Ti from Tu
	//(Ti = 2.2*Tu) so the oscillation gate and output timing below still
	//have a sensible reference to work with.
	mReferencePeriodSec(fallbackTiSec / 2.2),
	mReferenceAmplitude(DEFAULT_REFERENCE_AMPLITUDE),
	mLastError(0.0),
	mErrorDirection(0),
	mLastPeakTime(0),
	mHaveLastPeakTime(false),
	mLastPeakError(0.0),
	mHaveLastPeakError(false),
	mSettledErrorAvg(0.0),
	mSettledSamples(0),
	mSavePending(false),
	mLastSaveTime(0),
	mHaveLastSaveTime(false)
{
	clampGains();
	deriveOutputTimingFromReference();

	//if we've learned this room's behaviour before and it was persisted,
	//use it immediately instead of running (or re-running) auto-tune.
	loadPersistedState();

	if (mMode == Mode::RELAY_AUTOTUNE)
	{
		LOG(INFO) << "PID thermostat '" << mThermostatId << "': starting relay auto-tune";
	}
	else if (!mStartedFromPersistedState)
	{
		LOG(INFO) << "PID thermostat '" << mThermostatId << "': auto-tune disabled, starting with Kp=" << mKp
				<< ", Ti=" << mTiSec << "s, Td=" << mTdSec << "s";
	}
}

SelfLearningPidThermostat::~SelfLearningPidThermostat()
{
}

void SelfLearningPidThermostat::registerListener(ThermostatListenerIf* listener)
{
	mListener = listener;
}

void SelfLearningPidThermostat::unRegisterListener()
{
	mListener = nullptr;
}

void SelfLearningPidThermostat::setPointChanged(time_t timePoint, double setTemperature)
{
	bool bigJump = mHaveSetPoint && (std::fabs(setTemperature - mSetPoint) > SETPOINT_JUMP_THRESHOLD);
	mSetPoint = setTemperature;
	mHaveSetPoint = true;

	if (!bigJump) return;

	if (mMode == Mode::RELAY_AUTOTUNE)
	{
		//the target moved meaningfully while we were still characterising
		//the room: discard the partial measurement and start the relay
		//test over around the new setpoint.
		LOG(INFO) << "PID autotune: setpoint jumped to " << setTemperature << ", restarting relay test";
		mHalfCyclesSeen = 0;
		mConsistentPeriods = 0;
		mLastPeriodSec = 0.0;
		mHaveLastSwitchTime = false;
	}
	else
	{
		//a large setpoint step causes a legitimate large transient; don't
		//let the self-tuner misread it as a poorly damped oscillation or a
		//persistent steady-state offset.
		resetPeakTracking();
	}
}

void SelfLearningPidThermostat::temperatureChanged(time_t timePoint, double temperature)
{
	double dt = NOMINAL_SAMPLE_INTERVAL_SEC;
	if (mHaveLastSample)
	{
		double measuredDt = difftime(timePoint, mLastTimePoint);
		if (measuredDt > 0.0) dt = measuredDt;
	}
	mLastTimePoint = timePoint;
	mHaveLastSample = true;

	LOG(INFO) << "Temperature: " << temperature << ", setpoint: " << mSetPoint;

	//everything - PID math, auto-tune, self-learning, persistence, and
	//(re-)arming the heater's safety watchdog - happens right here, driven
	//only by this call. Nothing in this class ever runs on its own thread
	//or timer.
	if (mMode == Mode::RELAY_AUTOTUNE)
	{
		processRelayAutoTune(timePoint, temperature);
	}
	else
	{
		processPidControl(timePoint, temperature, dt);
	}

	mLastTemperature = temperature;
	mHaveLastTemperature = true;

	saveIfDue(timePoint);
}

void SelfLearningPidThermostat::processRelayAutoTune(time_t timePoint, double temperature)
{
	++mAutoTuneTicks;

	double halfBand = RELAY_HYSTERESIS / 2.0;
	if (mRelayOutputOn && temperature > mSetPoint + halfBand)
	{
		mRelayOutputOn = false;
		onRelaySwitch(timePoint, temperature);
	}
	else if (!mRelayOutputOn && temperature < mSetPoint - halfBand)
	{
		mRelayOutputOn = true;
		onRelaySwitch(timePoint, temperature);
	}

	if (temperature < mCycleMinTemp) mCycleMinTemp = temperature;
	if (temperature > mCycleMaxTemp) mCycleMaxTemp = temperature;

	driveHeater(mRelayOutputOn);

	if (mMode == Mode::RELAY_AUTOTUNE && mAutoTuneTicks >= AUTOTUNE_MAX_TICKS)
	{
		LOG(WARNING) << "PID autotune: did not converge within " << AUTOTUNE_MAX_TICKS
				<< " samples, falling back to the configured default gains";
		finishAutoTune();
	}
}

void SelfLearningPidThermostat::onRelaySwitch(time_t timePoint, double temperature)
{
	if (mRelayOutputOn)
	{
		//just switched OFF->ON: one full oscillation period has completed
		if (mHaveLastSwitchTime)
		{
			double period = difftime(timePoint, mLastSwitchTime);
			double amplitude = (mCycleMaxTemp - mCycleMinTemp) / 2.0;

			if (mHalfCyclesSeen >= AUTOTUNE_SKIP_HALF_CYCLES && period > 0.0
					&& amplitude > AUTOTUNE_MIN_USEFUL_AMPLITUDE)
			{
				if (mLastPeriodSec > 0.0
						&& std::fabs(period - mLastPeriodSec) < AUTOTUNE_PERIOD_TOLERANCE * mLastPeriodSec)
				{
					++mConsistentPeriods;
				}
				else
				{
					mConsistentPeriods = 0;
				}
				mLastPeriodSec = period;
				mLastAmplitude = amplitude;

				LOG(INFO) << "PID autotune: period=" << period << "s, amplitude=" << amplitude
						<< " (consistent=" << mConsistentPeriods << ")";

				if (mConsistentPeriods >= AUTOTUNE_REQUIRED_CONSISTENT_PERIODS)
				{
					finishAutoTune();
					return;
				}
			}
		}
		mLastSwitchTime = timePoint;
		mHaveLastSwitchTime = true;
		mCycleMinTemp = temperature;
		mCycleMaxTemp = temperature;
	}
	++mHalfCyclesSeen;
}

void SelfLearningPidThermostat::finishAutoTune()
{
	if (mConsistentPeriods >= AUTOTUNE_REQUIRED_CONSISTENT_PERIODS
			&& mLastPeriodSec > 0.0 && mLastAmplitude > 0.0)
	{
		//the heater duty toggles between 0 and 1 during the relay test,
		//which is equivalent to a symmetric relay of amplitude d=0.5 once
		//its mean (0.5) is removed.
		const double relayAmplitude = 0.5;
		double ku = (4.0 * relayAmplitude) / (PI * mLastAmplitude);
		double tu = mLastPeriodSec;

		//Tyreus-Luyben tuning: noticeably less aggressive/oscillatory than
		//classic Ziegler-Nichols, which suits a slow, actuator-limited
		//heating process.
		mKp = ku / 3.2;
		mTiSec = 2.2 * tu;
		mTdSec = tu / 6.3;
		mReferencePeriodSec = tu;
		mReferenceAmplitude = mLastAmplitude;
		clampGains();

		LOG(INFO) << "PID autotune converged: Tu=" << tu << "s, amplitude=" << mLastAmplitude
				<< " -> Kp=" << mKp << ", Ti=" << mTiSec << "s, Td=" << mTdSec << "s";
	}
	else
	{
		LOG(WARNING) << "PID autotune inconclusive, keeping fallback gains: Kp=" << mKp
				<< ", Ti=" << mTiSec << "s, Td=" << mTdSec << "s";
	}

	deriveOutputTimingFromReference();
	mCycleElapsedSec = 0.0;
	mCycleOnTimeTarget = 0.0;
	mOnTimeDebtSec = 0.0;

	mIntegral = 0.0;
	mAutoTuneTicks = 0;
	resetPeakTracking();
	mMode = Mode::PID_CONTROL;

	//persist either outcome: a converged tuning, so it never has to be
	//measured again, or an inconclusive one, so a room that can't be
	//characterised within AUTOTUNE_MAX_TICKS doesn't repeat that same
	//many-hour attempt on every future restart either. The online
	//self-tuner can still refine a fallback-derived starting point later.
	requestSave();
}

void SelfLearningPidThermostat::processPidControl(time_t timePoint, double temperature, double dt)
{
	double error = mSetPoint - temperature;

	double proportional = mKp * error;
	double ki = (mTiSec > 0.0) ? (mKp / mTiSec) : 0.0;
	double integralCandidate = mIntegral + ki * error * dt;

	double derivative = 0.0;
	if (mHaveLastTemperature && dt > 0.0)
	{
		//derivative on measurement, not on error, so a setpoint change
		//never causes a derivative "kick".
		derivative = -mKp * mTdSec * (temperature - mLastTemperature) / dt;
	}

	//clamped anti-windup: only let the integral term keep growing if doing
	//so would not push an already-saturated output further into saturation
	double provisionalOutput = proportional + integralCandidate + derivative;
	bool saturatedHigh = provisionalOutput > 1.0;
	bool saturatedLow = provisionalOutput < 0.0;
	bool integralGrowsIntoSaturation =
			(saturatedHigh && integralCandidate > mIntegral) ||
			(saturatedLow && integralCandidate < mIntegral);
	if (!integralGrowsIntoSaturation)
	{
		mIntegral = integralCandidate;
	}

	double output = clamp(proportional + mIntegral + derivative, 0.0, 1.0);

	adaptGains(timePoint, error, output);

	updateDutyOutput(output, dt);
}

void SelfLearningPidThermostat::adaptGains(time_t timePoint, double error, double output)
{
	//--- peak (turning point) detection on the error signal, used to judge
	//    how well damped the closed loop is ---
	double delta = error - mLastError;
	int direction = 0;
	if (delta > PEAK_NOISE_MARGIN) direction = 1;
	else if (delta < -PEAK_NOISE_MARGIN) direction = -1;

	if (direction != 0)
	{
		if (mErrorDirection != 0 && direction != mErrorDirection)
		{
			//the error just reversed direction: mLastError (the previous
			//sample) was a local peak.
			onErrorPeak(timePoint, mLastError);
		}
		mErrorDirection = direction;
	}
	mLastError = error;

	//--- persistent steady-state offset tracking, used to correct Ti ---
	//only trust this while the loop is close to the setpoint and the
	//output isn't saturated (a saturated output can't reduce the error no
	//matter how good Ti is, so it says nothing about the integral time).
	if (std::fabs(error) < SETTLED_ERROR_BAND && output > 0.02 && output < 0.98)
	{
		mSettledErrorAvg += BIAS_FILTER_ALPHA * (error - mSettledErrorAvg);
		++mSettledSamples;
		if (mSettledSamples >= BIAS_MIN_SAMPLES && std::fabs(mSettledErrorAvg) > BIAS_TRIGGER)
		{
			mTiSec *= TI_ADAPT_DOWN_FACTOR;
			clampGains();
			LOG(INFO) << "PID self-tune: persistent offset (" << mSettledErrorAvg
					<< "), shortening Ti to " << mTiSec << "s";
			mSettledSamples = 0;
			mSettledErrorAvg = 0.0;
			requestSave();
		}
	}
	else
	{
		mSettledSamples = 0;
		mSettledErrorAvg = 0.0;
	}
}

void SelfLearningPidThermostat::onErrorPeak(time_t timePoint, double peakError)
{
	double significantPeakThreshold = std::max(MIN_PEAK_MAGNITUDE_FLOOR,
			SIGNIFICANT_PEAK_FRACTION_OF_REFERENCE_AMPLITUDE * mReferenceAmplitude);
	if (std::fabs(peakError) < significantPeakThreshold) return;

	if (mHaveLastPeakError && mHaveLastPeakTime && (peakError * mLastPeakError < 0.0))
	{
		double periodSincePeak = difftime(timePoint, mLastPeakTime);

		//Only two consecutive peaks spaced apart by a meaningful fraction
		//of the process's ultimate period look like a genuine, poorly
		//damped closed-loop oscillation. The time-proportioned output can
		//only switch once per output cycle, so it produces its own small
		//ripple around the setpoint at a period far below Tu; that ripple
		//is an artifact of the actuator's resolution, not a sign the gain
		//is too high, and adapting on it would just grind Kp down to its
		//floor for no reason.
		double minOscillationPeriod = std::max(MIN_OSCILLATION_PERIOD_FRACTION_OF_TU * mReferencePeriodSec,
				MIN_OSCILLATION_PERIOD_CYCLES * mCycleLengthSec);
		if (periodSincePeak >= minOscillationPeriod)
		{
			//two consecutive peaks on opposite sides of the setpoint: a
			//real half-oscillation. Its decay ratio tells us whether the
			//loop is too aggressive (poorly damped) or too timid
			//(sluggish), and we nudge Kp back towards the classic
			//quarter-amplitude-damping target.
			double decayRatio = std::fabs(peakError) / std::fabs(mLastPeakError);

			if (decayRatio > OSCILLATION_DECAY_RATIO_HIGH)
			{
				mKp *= GAIN_ADAPT_DOWN_FACTOR;
				clampGains();
				LOG(INFO) << "PID self-tune: oscillatory response (decay=" << decayRatio
						<< ", period=" << periodSincePeak << "s), reducing Kp to " << mKp;
				requestSave();
			}
			else if (decayRatio < OSCILLATION_DECAY_RATIO_LOW)
			{
				mKp *= GAIN_ADAPT_UP_FACTOR;
				clampGains();
				LOG(INFO) << "PID self-tune: sluggish response (decay=" << decayRatio
						<< ", period=" << periodSincePeak << "s), increasing Kp to " << mKp;
				requestSave();
			}
		}
	}

	mLastPeakError = peakError;
	mHaveLastPeakError = true;
	mLastPeakTime = timePoint;
	mHaveLastPeakTime = true;
}

void SelfLearningPidThermostat::updateDutyOutput(double duty, double dt)
{
	//classic time-proportioning control: rather than re-deciding on/off
	//from scratch on every single sample (which would switch a real
	//relay/pump/valve far more often than a slow room could ever need),
	//the PID's [0..1] duty cycle is only sampled once per output cycle,
	//and held for the whole cycle. mCycleLengthSec/mMinOnTimeSec are
	//derived from the room's own identified ultimate period (see
	//deriveOutputTimingFromReference()), so this naturally scales to
	//however fast or slow this particular room actually is. Still driven
	//purely by the dt derived from temperatureChanged()'s timestamps - no
	//timer of its own.
	duty = clamp(duty, 0.0, 1.0);

	mCycleElapsedSec += dt;
	if (mCycleElapsedSec >= mCycleLengthSec)
	{
		//new cycle: carry over any overshoot so timing doesn't drift if dt
		//doesn't divide the cycle length evenly.
		mCycleElapsedSec -= mCycleLengthSec;
		if (mCycleElapsedSec < 0.0) mCycleElapsedSec = 0.0;

		mOnTimeDebtSec += duty * mCycleLengthSec;

		if (mOnTimeDebtSec >= mMinOnTimeSec)
		{
			mCycleOnTimeTarget = std::min(mOnTimeDebtSec, mCycleLengthSec);
			mOnTimeDebtSec -= mCycleOnTimeTarget;
		}
		else
		{
			//the owed on-time is too small to be worth actually switching
			//the heater on for - skip this cycle's heat and let the debt
			//keep building. It is never lost, only delayed, so the
			//long-run average duty this delivers is still correct.
			mCycleOnTimeTarget = 0.0;
		}
	}

	bool on = mCycleElapsedSec < mCycleOnTimeTarget;
	driveHeater(on);
}

void SelfLearningPidThermostat::driveHeater(bool on)
{
	mHeaterCommandedOn = on;
	if (!mListener) return;

	//Called unconditionally, every single temperatureChanged() tick - even
	//when the commanded state doesn't change - because the heater hardware
	//auto-off's after ~60s if it isn't re-armed. As long as temperature
	//reports keep arriving at their normal ~30s cadence this re-arms the
	//watchdog well within its timeout.
	if (on)
	{
		mListener->heaterOn();
	}
	else
	{
		mListener->heaterOff();
	}
}

void SelfLearningPidThermostat::resetPeakTracking()
{
	mErrorDirection = 0;
	mHaveLastPeakError = false;
	mHaveLastPeakTime = false;
	mSettledSamples = 0;
	mSettledErrorAvg = 0.0;
}

void SelfLearningPidThermostat::clampGains()
{
	mKp = clamp(mKp, KP_MIN, KP_MAX);
	mTiSec = clamp(mTiSec, TI_MIN_SEC, TI_MAX_SEC);
	mTdSec = clamp(mTdSec, TD_MIN_SEC, TD_MAX_SEC);
}

void SelfLearningPidThermostat::deriveOutputTimingFromReference()
{
	mCycleLengthSec = clamp(mReferencePeriodSec / CYCLE_LENGTH_TU_FRACTION, MIN_CYCLE_LENGTH_SEC, MAX_CYCLE_LENGTH_SEC);
	mMinOnTimeSec = clamp(mCycleLengthSec * MIN_ON_TIME_FRACTION_OF_CYCLE, MIN_ON_TIME_FLOOR_SEC, MIN_ON_TIME_CEIL_SEC);
}

void SelfLearningPidThermostat::loadPersistedState()
{
	if (!mStorage) return;

	PidLearnedState state;
	if (!mStorage->loadPidState(mThermostatId, state)) return;

	mKp = state.kp;
	mTiSec = state.tiSec;
	mTdSec = state.tdSec;
	mReferencePeriodSec = state.referencePeriodSec;
	mReferenceAmplitude = state.referenceAmplitude;
	clampGains();
	deriveOutputTimingFromReference();

	mMode = Mode::PID_CONTROL;
	mStartedFromPersistedState = true;

	LOG(INFO) << "PID thermostat '" << mThermostatId << "': restored learned state, skipping auto-tune "
			<< "(Kp=" << mKp << ", Ti=" << mTiSec << "s, Td=" << mTdSec << "s, cycle=" << mCycleLengthSec << "s)";
}

void SelfLearningPidThermostat::requestSave()
{
	if (mStorage) mSavePending = true;
}

void SelfLearningPidThermostat::saveIfDue(time_t timePoint)
{
	if (!mStorage || !mSavePending) return;

	if (mHaveLastSaveTime && difftime(timePoint, mLastSaveTime) < MIN_SAVE_INTERVAL_SEC)
	{
		return; //save is pending but throttled; will be picked up on a later tick
	}

	PidLearnedState state;
	state.kp = mKp;
	state.tiSec = mTiSec;
	state.tdSec = mTdSec;
	state.referencePeriodSec = mReferencePeriodSec;
	state.referenceAmplitude = mReferenceAmplitude;
	mStorage->savePidState(mThermostatId, state);

	mSavePending = false;
	mLastSaveTime = timePoint;
	mHaveLastSaveTime = true;
}

void SelfLearningPidThermostat::forgetLearnedState()
{
	mMode = Mode::RELAY_AUTOTUNE;
	mStartedFromPersistedState = false;

	mRelayOutputOn = false;
	mHaveLastSwitchTime = false;
	mHalfCyclesSeen = 0;
	mConsistentPeriods = 0;
	mLastPeriodSec = 0.0;
	mLastAmplitude = 0.0;
	mAutoTuneTicks = 0;

	mIntegral = 0.0;
	resetPeakTracking();

	LOG(INFO) << "PID thermostat '" << mThermostatId << "': learned state cleared, restarting auto-tune";
}

double SelfLearningPidThermostat::clamp(double value, double lo, double hi)
{
	if (value < lo) return lo;
	if (value > hi) return hi;
	return value;
}

} /* namespace LogicNs */
