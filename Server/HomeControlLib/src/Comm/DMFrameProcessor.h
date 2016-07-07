/*
 * DMFrameProcessor.h
 *
 *  Created on: Jul 7, 2016
 *      Author: koen
 */

#ifndef COMM_DMFRAMEPROCESSOR_H_
#define COMM_DMFRAMEPROCESSOR_H_
#include "SerialListenerIf.h"
#include <vector>
#include <stdint.h>
#include <mutex>


namespace CommNs {
class DMFrameListenerIf;

class DMFrameProcessor: public SerialListenerIf
{
public:
	DMFrameProcessor();
	virtual ~DMFrameProcessor();

	void registerFrameListener(DMFrameListenerIf* listener);
	void unRegisterFrameListener();

	//SerialListenerIf
	void receiveData(const std::vector<uint8_t>& data);
private:
    std::vector<uint8_t> mFrameBuffer;
    std::mutex mDataMutex;
    DMFrameListenerIf* mListener;

    void processFrameBuffer();
};

} /* namespace CommNs */

#endif /* COMM_DMFRAMEPROCESSOR_H_ */
