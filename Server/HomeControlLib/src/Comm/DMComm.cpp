/*
 * DMComm.cpp
 *
 *  Created on: Jul 9, 2016
 *      Author: koen
 */

#include <Comm/DMComm.h>
#include "Comm/DMFrameProcessorIf.h"
#include "Comm/DMMessages.h"
#include "glog/logging.h"
#include <iomanip>
#include <sstream>

namespace CommNs {

DMComm::DMComm(DMFrameProcessorIf* frameProcessor):
	mFrameProcessor(frameProcessor),
	mFrameId(1),
	mSynchronousFrameId(0),
    mWaitForResponseCondVar(),
    mResponseReceived(),
	mConditionVarMutex(),
	mSN()
{
	if (mFrameProcessor)
	{
		mFrameProcessor->registerFrameListener(this);
	}
	init();
}

DMComm::~DMComm()
{
	if (mFrameProcessor)
	{
		mFrameProcessor->unRegisterFrameListener();
	}
}

std::string DMComm::snString()
{
	std::stringstream ss;
	bool first = true;
	for(auto val: mSN)
	{
		if (!first)
		{
			ss << ":";
		}
		ss << std::hex << std::uppercase <<  std::setfill('0') << std::setw(2) <<  (int) val;
		first = false;
	}
	return ss.str();
}

void DMComm::sendATCmd(const std::string& atCmd)
{
	if (mFrameProcessor)
	{
		std::vector<uint8_t> sendCmd;
		sendCmd.push_back(0x08);
		sendCmd.push_back(mFrameId++);
		sendCmd.insert(sendCmd.end(), atCmd.begin(), atCmd.end());

		mFrameProcessor->sendData(sendCmd);
	}

	if (mFrameId == 0) mFrameId = 1;
}

DMMessageIf* DMComm::sendATCmd(const std::string& atCmd, int timeOutMilliseconds)
{
	mResponseReceived = false;
	mSynchronousFrameId = mFrameId;
	mReceivedMessage = nullptr;
	sendATCmd(atCmd);

	auto until = std::chrono::system_clock::now() +std::chrono::milliseconds(timeOutMilliseconds);
	std::unique_lock<std::mutex> lock(mConditionVarMutex);
	mWaitForResponseCondVar.wait_until(lock, until,[&]{return (bool)mResponseReceived;});
	if (mResponseReceived)
	{
		VLOG(1) << "Response received for command: " << atCmd;
		return mReceivedMessage;
	}
	mSynchronousFrameId = 0;

	return nullptr;
}

void DMComm::sendMessage(DMMessageIf* message)
{

}

void DMComm::receiveFrame(const std::vector<uint8_t>& data)
{
	if (VLOG_IS_ON(2))
	{
		printFrame(data);
	}

	if (data.size() < 1) return;
	bool synchronousResponse = false;
	const uint8_t frameType = data[0];
	const uint8_t frameId = data[1];
	if (mSynchronousFrameId == frameId)
	{
		synchronousResponse = true;
	}

	DMMessageIf* received = nullptr;

	switch(frameType)
	{
		case 0x88:
		{
			std::string atCmd(data.begin() + 2, data.begin() + 4);
			LOG(INFO) << "AT Command response received, command: " << atCmd;
			if (atCmd == "SH")
			{
				LOG(INFO) << "SH received";
				received = new AtResponse_SN(DMMessageType::ATResponse_SH);
				std::copy(data.begin() + 5, data.begin() + 8, std::back_inserter( ((AtResponse_SN*)received)->SN));
			}
			if (atCmd == "SL")
			{
				LOG(INFO) << "SL received";
				received = new AtResponse_SN(DMMessageType::ATResponse_SL);
				std::copy(data.begin() + 5, data.begin() + 9, std::back_inserter( ((AtResponse_SN*)received)->SN));
			}
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

	if (synchronousResponse)
	{
		mResponseReceived = true;
		mReceivedMessage = received;
		mWaitForResponseCondVar.notify_one();
	}
	else
	{

	}
}

void DMComm::printFrame(const std::vector<uint8_t>& data)
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
	VLOG(2) << "Received frame: " << ss.str();
}

void DMComm::init()
{
	DMMessageIf* received = sendATCmd("SH", 100);
	if(AtResponse_SN* sn = dynamic_cast<AtResponse_SN*> (received))
	{
		mSN.insert(mSN.end(), sn->SN.begin(), sn->SN.end());
		delete sn;
	}

	received = sendATCmd("SL", 100);
	if(AtResponse_SN* sn = dynamic_cast<AtResponse_SN*> (received))
	{
		mSN.insert(mSN.end(), sn->SN.begin(), sn->SN.end());
		delete sn;
	}

	if (mSN.size() == 7)
	{
		LOG(INFO) << "Device initialised, SN: " << snString();
	}
	else
	{
		LOG(ERROR) << "Device not initalised, SN length: " << mSN.size();
	}

}

} /* namespace CommNs */
