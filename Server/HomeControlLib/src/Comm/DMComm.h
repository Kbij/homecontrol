/*
 * DMComm.h
 *
 *  Created on: Jul 9, 2016
 *      Author: koen
 */

#ifndef COMM_DMCOMM_H_
#define COMM_DMCOMM_H_
#include "Comm/DMFrameListenerIf.h"
#include <condition_variable>
#include <atomic>

namespace CommNs {
class DMFrameProcessorIf;
class DMMessageIf;

class DMComm: public CommNs::DMFrameListenerIf
{
public:
	DMComm(DMFrameProcessorIf* frameProcessor);
	virtual ~DMComm();


	std::string snString();

	void sendATCmd(const std::string& atCmd);
	DMMessageIf* sendATCmd(const std::string& atCmd, int timeOutMilliseconds);
	void sendMessage(DMMessageIf* message);

	//DMFrameListenerIf
	void receiveFrame(const std::vector<uint8_t>& data);
private:
	DMFrameProcessorIf* mFrameProcessor;
	uint8_t mFrameId;
	uint8_t mSynchronousFrameId;
//	uint8_t mExpectedResponseFrame;
//	std::string mExpectedATResponse;
    std::condition_variable mWaitForResponseCondVar;
    std::atomic_bool mResponseReceived;
	std::mutex mConditionVarMutex;
	DMMessageIf* mReceivedMessage;
	std::vector<uint8_t> mSN;

	void printFrame(const std::vector<uint8_t>& data);
	void init();
};

} /* namespace CommNs */

#endif /* COMM_DMCOMM_H_ */
