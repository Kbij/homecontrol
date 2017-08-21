/*
 * DMComm.h
 *
 *  Created on: Jul 9, 2016
 *      Author: koen
 */

#ifndef COMM_DMCOMM_H_
#define COMM_DMCOMM_H_
#include "Comm/DMFrameListenerIf.h"
#include "Comm/DMCommIf.h"
#include <condition_variable>
#include <atomic>
#include <vector>
#include <stdint.h>
#include <mutex>

namespace CommNs {
class DMFrameProcessorIf;
class DMMessageIf;
class DMCommListenerIf;

class DMComm: public DMCommIf, public CommNs::DMFrameListenerIf
{
public:
	DMComm(DMFrameProcessorIf* frameProcessor, uint8_t channel, std::vector<uint8_t> id);
	virtual ~DMComm();

	void registerListener(DMCommListenerIf* listener);
	void unRegisterListener(DMCommListenerIf* listener);

	std::string addressString();

	void sendATCmd(const std::string& atCmd, std::vector<uint8_t> parameters);
	DMMessageIf* sendATCmd(const std::string& atCmd, std::vector<uint8_t> parameters, int timeOutMilliseconds);
	void sendMessage(DMMessageIf* message);
	DMMessageIf* sendMessage(DMMessageIf* message, int timeOutMilliseconds);

	//DMFrameListenerIf
	void receiveFrame(const std::vector<uint8_t>& data);

	void init();
private:
	DMFrameProcessorIf* mFrameProcessor;
	const uint8_t mChannel;
	const std::vector<uint8_t> mId;
	DMCommListenerIf* mListener;
	uint8_t mFrameId;
	uint8_t mSynchronousFrameId;
    std::condition_variable mWaitForResponseCondVar;
    std::atomic_bool mResponseReceived;
	std::mutex mConditionVarMutex;
	DMMessageIf* mReceivedMessage;
	std::vector<uint8_t> mSN;
	std::recursive_mutex mDataMutex;

	void printFrame(const std::vector<uint8_t>& data);

};

} /* namespace CommNs */

#endif /* COMM_DMCOMM_H_ */
