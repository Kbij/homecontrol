/*
 * DMFrameProcessor.cpp
 *
 *  Created on: Jul 7, 2016
 *      Author: koen
 */

#include <Comm/DMFrameProcessor.h>
#include "Comm/DMFrameListenerIf.h"
#include "glog/logging.h"
#include <iterator>
#include <algorithm>

namespace
{
const int HEADER_LENGTH = 3;
const int LENGTH_POSITION = 1;
const int CRC_LENGTH = 1;
}
namespace CommNs {

DMFrameProcessor::DMFrameProcessor():
	mFrameBuffer(),
	mDataMutex(),
	mListener(nullptr)
{

}

DMFrameProcessor::~DMFrameProcessor()
{
}

void DMFrameProcessor::registerFrameListener(DMFrameListenerIf* listener)
{
	mListener = listener;
}

void DMFrameProcessor::unRegisterFrameListener()
{
	mListener = nullptr;
}

void DMFrameProcessor::receiveData(const std::vector<uint8_t>& data)
{
	std::lock_guard<std::mutex> lk_guard(mDataMutex);
	mFrameBuffer.insert(mFrameBuffer.end(), data.begin(), data.end());

	processFrameBuffer();
}

void DMFrameProcessor::processFrameBuffer()
{
	bool packetFound = true;
	while (packetFound && mFrameBuffer.size() > 0)
	{
		packetFound = false;
		if (mFrameBuffer.size() < HEADER_LENGTH)
		{ // Not received the full buffer; return
			VLOG(3) << "Not a full frame received, buffer size: " << mFrameBuffer.size();
			return;
		}

		auto itStartPosition = std::find(mFrameBuffer.begin(), mFrameBuffer.end(), 0x7E);
		if (itStartPosition == mFrameBuffer.end())
		{ //Start not found; return
			if (mFrameBuffer.size() > 20)
			{	//Avoid a buffer build-up
				LOG(ERROR) << "While searching for start, buffer cleared. Size: " << mFrameBuffer.size();
				mFrameBuffer.clear();
			}
			return;
		}
		auto startPosition = itStartPosition - mFrameBuffer.begin();
		int dataLength = mFrameBuffer[startPosition + LENGTH_POSITION] * 256 +
						 mFrameBuffer[startPosition + LENGTH_POSITION + 1];
		if (mFrameBuffer.size() < (size_t) dataLength + HEADER_LENGTH + CRC_LENGTH)
		{ // Not all data received
			VLOG(3) << "Not all data received, buffer size: " << mFrameBuffer.size();
			return;
		}

		VLOG(3) << "Full frame received. buffer size: " << mFrameBuffer.size() << ", packet length: " << dataLength;
		std::vector<uint8_t> frame(itStartPosition +  HEADER_LENGTH, itStartPosition + HEADER_LENGTH + dataLength);

		uint8_t calculatedCrc = std::accumulate(frame.begin(), frame.end(), 0);
		calculatedCrc = calculatedCrc + *(itStartPosition + HEADER_LENGTH + dataLength);
		if (calculatedCrc == 0xFF)
		{
			if (mListener)
			{
				mListener->receiveFrame(frame);
			}
		}
		else
		{
			LOG(ERROR) << "Invalid Crc";
		}

		mFrameBuffer.erase(mFrameBuffer.begin(), itStartPosition + HEADER_LENGTH + dataLength + CRC_LENGTH);
		packetFound = true;
		VLOG(3) << "Buffer size: " << mFrameBuffer.size();
	}
}
} /* namespace CommNs */
