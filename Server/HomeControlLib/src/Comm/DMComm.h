/*
 * DMComm.h
 *
 *  Created on: Jul 9, 2016
 *      Author: koen
 */

#ifndef COMM_DMCOMM_H_
#define COMM_DMCOMM_H_
#include "Comm/DMFrameListenerIf.h"

namespace CommNs {
class DMFrameProcessorIf;

class DMComm: public CommNs::DMFrameListenerIf
{
public:
	DMComm(DMFrameProcessorIf* frameProcessor);
	virtual ~DMComm();

	//DMFrameListenerIf
	void receiveFrame(const std::vector<uint8_t>& data);
private:
	DMFrameProcessorIf* mFrameProcessor;
};

} /* namespace CommNs */

#endif /* COMM_DMCOMM_H_ */
