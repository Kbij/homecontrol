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
#include <iomanip>
#include <sstream>

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
	mListener(nullptr),
	mFrameId(1)
{
	if (mSerial)
	{
		mSerial->registerSerialListener(this);
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
		printFrame("Send Raw Frame: ", sendFrame);
		mSerial->writeData(sendFrame);
	}
}

void DMFrameProcessor::receiveData(const std::vector<uint8_t>& data)
{
	std::lock_guard<std::mutex> lk_guard(mDataMutex);
	VLOG(20) << "Receive size : " << data.size();
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
			VLOG(20) << "Not a full frame received, buffer size: " << mFrameBuffer.size();
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
			VLOG(20) << "Start not found, first byte: " << (int) mFrameBuffer[0] << ", buffer: " << mFrameBuffer;
			return;
		}
		auto startPosition = itStartPosition - mFrameBuffer.begin();
		int dataLength = mFrameBuffer[startPosition + LENGTH_POSITION] * 256 +
						 mFrameBuffer[startPosition + LENGTH_POSITION + 1];
		VLOG(20) << "dataLength: " << dataLength;
		if (mFrameBuffer.size() < (size_t) dataLength + HEADER_LENGTH + CRC_LENGTH)
		{ // Not all data received
			VLOG(20) << "Not all data received, buffer size: " << mFrameBuffer.size();
			return;
		}

		VLOG(20) << "Full frame received. buffer size: " << mFrameBuffer.size() << ", packet length: " << dataLength;
		printFrame("Received Raw Frame: ", std::vector<uint8_t>(itStartPosition, itStartPosition + HEADER_LENGTH + dataLength + 1)); // +1: CRC
		std::vector<uint8_t> frame(itStartPosition +  HEADER_LENGTH, itStartPosition + HEADER_LENGTH + dataLength);

		uint8_t calculatedCrc = std::accumulate(frame.begin(), frame.end(), 0);
		calculatedCrc = calculatedCrc + *(itStartPosition + HEADER_LENGTH + dataLength);
		if (calculatedCrc == 0xFF)
		{
			if (mListener)
			{
				mListener->receiveFrame(frame);
			}
			mFrameBuffer.erase(mFrameBuffer.begin(), itStartPosition + HEADER_LENGTH + dataLength + CRC_LENGTH);
			packetFound = true;
		}
		else
		{
			LOG(ERROR) << "Invalid Crc";
			mFrameBuffer.clear();
		}

		VLOG(20) << "Buffer size: " << mFrameBuffer.size();
	}
}

void DMFrameProcessor::printFrame(const std::string& name, const std::vector<uint8_t>& data)
{
	std::stringstream ss;
	bool first = true;
	for(auto val: data)
	{
		if (!first)
		{
			ss << ", ";
		}
		ss << "0x" << std::hex << std::uppercase <<  std::setfill('0') << std::setw(2) <<  (int) val;
		first = false;
	}
	VLOG(21) << name << ss.str();
}
} /* namespace CommNs */
