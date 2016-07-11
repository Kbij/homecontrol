/*
 * DMFrameProcessorIf.h
 *
 *  Created on: Jul 10, 2016
 *      Author: koen
 */

#ifndef COMM_DMFRAMEPROCESSORIF_H_
#define COMM_DMFRAMEPROCESSORIF_H_
#include <vector>
#include <stdint.h>

namespace CommNs {
class DMFrameListenerIf;

class DMFrameProcessorIf
{
public:
	virtual ~DMFrameProcessorIf() {};

	virtual void registerFrameListener(DMFrameListenerIf* listener) = 0;
	virtual void unRegisterFrameListener() = 0;

	virtual void sendData(const std::vector<uint8_t>& data) = 0;
};

} /* namespace CommNs */



#endif /* COMM_DMFRAMEPROCESSORIF_H_ */
