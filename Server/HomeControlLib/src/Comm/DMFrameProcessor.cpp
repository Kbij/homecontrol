/*
 * DMFrameProcessor.cpp
 *
 *  Created on: Jul 7, 2016
 *      Author: koen
 */

#include <Comm/DMFrameProcessor.h>
#include "Comm/DMFrameListenerIf.h"
#include "Comm/SerialIf.h"
#include "glog/stl_logging.h"
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

DMFrameProcessor::DMFrameProcessor(SerialIf* serial):
	mSerial(serial),
	mFrameBuffer(),
	mDataMutex(),
	mListener(nullptr)
{
	if (mSerial)
	{
		mSerial->registerSerialListener(this);
	//	mSerial->writeData({'A', 'T', ' ', '1', 0x0D});
	}
}

DMFrameProcessor::~DMFrameProcessor()
{
	if (mSerial)
	{
		mSerial->unRegisterSerialListener();
	}
}

void DMFrameProcessor::registerFrameListener(DMFrameListenerIf* listener)
{
	mListener = listener;
}

void DMFrameProcessor::unRegisterFrameListener()
{
	mListener = nullptr;
}

void DMFrameProcessor::sendATCmd(const std::string& atCmd)
{
	std::vector<uint8_t> data;
	data.push_back(0x08);
	data.push_back(0x00);
	data.insert(data.end(), atCmd.begin(), atCmd.end());

	sendData(data);
}

void DMFrameProcessor::sendData(const std::vector<uint8_t>& data)
{
	if (mSerial)
	{
		std::vector<uint8_t> sendFrame;
		sendFrame.push_back(0x7E);
		sendFrame.push_back(data.size() / 256);
		sendFrame.push_back(data.size() % 256);
		sendFrame.insert(sendFrame.end(), data.begin(), data.end());
		uint8_t calculatedCrc = std::accumulate(data.begin(), data.end(), 0);
		calculatedCrc = 0xFF - calculatedCrc;
		sendFrame.push_back(calculatedCrc);

		mSerial->writeData(sendFrame);
	}
}

void DMFrameProcessor::receiveData(const std::vector<uint8_t>& data)
{
	std::lock_guard<std::mutex> lk_guard(mDataMutex);
	VLOG(1) << "Received (" << data.size() << ") " << data;
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
			if (mFrameBuffer.size() > 256)
			{	//Avoid a buffer build-up
				LOG(ERROR) << "While searching for start, buffer cleared. Size: " << mFrameBuffer.size();
				mFrameBuffer.clear();
			}
			VLOG(1) << "Start not found, first byte: " << (int) mFrameBuffer[0] << ", buffer: " << mFrameBuffer;
			return;
		}
		auto startPosition = itStartPosition - mFrameBuffer.begin();
		int dataLength = mFrameBuffer[startPosition + LENGTH_POSITION] * 256 +
						 mFrameBuffer[startPosition + LENGTH_POSITION + 1];
		VLOG(1) << "dataLength: " << dataLength;
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
