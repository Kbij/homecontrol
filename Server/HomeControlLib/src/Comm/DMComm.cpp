/*
 * DMComm.cpp
 *
 *  Created on: Jul 9, 2016
 *      Author: koen
 */

#include <Comm/DMComm.h>
#include "glog/logging.h"

namespace CommNs {

DMComm::DMComm(DMFrameProcessorIf* frameProcessor):
	mFrameProcessor(frameProcessor)
{
}

DMComm::~DMComm()
{
}

void DMComm::receiveFrame(const std::vector<uint8_t>& data)
{
	if (data.size() < 1) return;
	switch(data[0])
	{
		case 0x88:
		{
			LOG(INFO) << "AT Command response received";
			break;
		}
		case 0x8A:
		{
			LOG(INFO) << "Modem status received";
			break;
		}
		default:
		{
			LOG(INFO) << "Unknown response received: " << (int) data[0];
		}
	}

}

} /* namespace CommNs */
