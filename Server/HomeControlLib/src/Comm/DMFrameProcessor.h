/*
 * DMFrameProcessor.h
 *
 *  Created on: Jul 7, 2016
 *      Author: koen
 */

#ifndef COMM_DMFRAMEPROCESSOR_H_
#define COMM_DMFRAMEPROCESSOR_H_
#include "SerialListenerIf.h"
#include "DMFrameProcessorIf.h"
#include <vector>
#include <stdint.h>
#include <mutex>


namespace CommNs {
class DMFrameListenerIf;
class SerialIf;

class DMFrameProcessor: public DMFrameProcessorIf, public SerialListenerIf
{
public:
	DMFrameProcessor(SerialIf* serial);
	virtual ~DMFrameProcessor();

	void registerFrameListener(DMFrameListenerIf* listener);
	void unRegisterFrameListener();

	void sendData(const std::vector<uint8_t>& data);

	//SerialListenerIf
	void receiveData(const std::vector<uint8_t>& data);

private:
	SerialIf* mSerial;
    std::vector<uint8_t> mFrameBuffer;
    std::mutex mDataMutex;
    DMFrameListenerIf* mListener;
    uint8_t mFrameId;

    void processFrameBuffer();
};

} /* namespace CommNs */

#endif /* COMM_DMFRAMEPROCESSOR_H_ */
