/*
 * RoomControl.h
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#ifndef LOGIC_ROOMCONTROL_H_
#define LOGIC_ROOMCONTROL_H_

#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace LogicNs {
class RoomListenerIf;

class RoomControl
{
public:
	RoomControl(const std::string& roomId, const std::string& roomName, RoomListenerIf* roomListener);
	virtual ~RoomControl();

	std::string roomId() const {return mRoomId; };
	std::string roomName() const {return mRoomName; };
	void roomTemperature(double temperature);
	double roomTemperature() {return mRoomTemperature;};
	double setTemperature() {return mSetTemperature;};
	void setPointUp();
	void setPointDown();
private:
	void startWorkerThread();
	void stopWorkerThread();
	void workerThread();

	const std::string mRoomId;
	const std::string mRoomName;
	RoomListenerIf* mRoomListener;
	double mRoomTemperature;
	double mSetTemperature;
	std::thread* mWorkerThread;
	bool mWorkerThreadRunning;
	std::atomic_int mUpRequested;
	std::atomic_int mDownRequested;
	std::atomic_bool mTempReceived;
    std::condition_variable mWaitForWorkCondVar;
    std::atomic_int mWorkReceived;
	std::mutex mConditionVarMutex;
	std::recursive_mutex mDataMutex;
};

} /* namespace LogicNs */

#endif /* LOGIC_ROOMCONTROL_H_ */
