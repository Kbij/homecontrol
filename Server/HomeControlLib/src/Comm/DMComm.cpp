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

void DMComm::sendATCmd(const std::string& atCmd, std::vector<uint8_t> parameters)
{
	if (mFrameProcessor)
	{
		LOG(INFO) << "Send AT: " << atCmd;
		std::vector<uint8_t> sendCmd;
		sendCmd.push_back(0x08);
		sendCmd.push_back(mFrameId++);
		sendCmd.insert(sendCmd.end(), atCmd.begin(), atCmd.end());
		sendCmd.insert(sendCmd.end(), parameters.begin(), parameters.end());

		mFrameProcessor->sendData(sendCmd);
	}

	if (mFrameId == 0) mFrameId = 1;
}

DMMessageIf* DMComm::sendATCmd(const std::string& atCmd, std::vector<uint8_t> parameters, int timeOutMilliseconds)
{
	mResponseReceived = false;
	mSynchronousFrameId = mFrameId;
	mReceivedMessage = nullptr;
	sendATCmd(atCmd, parameters);

	auto until = std::chrono::system_clock::now() +std::chrono::milliseconds(timeOutMilliseconds);
	std::unique_lock<std::mutex> lock(mConditionVarMutex);
	mWaitForResponseCondVar.wait_until(lock, until,[&]{return (bool)mResponseReceived;});
	if (mResponseReceived)
	{
		VLOG(10) << "Response received for command: " << atCmd;
		return mReceivedMessage;
	}
	else
	{
		LOG(ERROR) << "No response received for cmd: " << atCmd << ", timeout: " << timeOutMilliseconds;
	}
	mSynchronousFrameId = 0;

	return nullptr;
}

void DMComm::sendMessage(DMMessageIf* message)
{
	if (message)
	{
		if (mFrameProcessor)
		{
			LOG(INFO) << "Send message: " << message->toString();
			mFrameProcessor->sendData(message->serialise(mFrameId++));
		}

		delete message;
	}
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
			VLOG(10) << "AT Command response received, command: " << atCmd;
			received = new AtResponse(atCmd, std::vector<uint8_t>(data.begin() + 5, data.end()));
			break;
		}
		case 0x8A:
		{
			VLOG(10) << "Modem status received";
			break;
		}
		case 0x8B:
		{
			VLOG(10) << "TX Status received";
			received = new TxStatusFrame(std::vector<uint8_t>(data.begin(), data.end()));
			break;
		}
		default:
		{
			LOG(INFO) << "Unknown response received, frameType: " << "0x" << std::hex << std::uppercase <<  std::setfill('0') << std::setw(2) << (int) frameType;
		}
	}

	if (received)
	{
		VLOG(10) << "Received: " << received->toString();
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
	VLOG(10) << "Received frame: " << ss.str();
}

void DMComm::init()
{
	DMMessageIf* received = sendATCmd("SH", {}, 100);
	if (received)
	{
		if(AtResponse* sn = dynamic_cast<AtResponse*> (received))
		{
			if (sn->cmd() == "SH") mSN.insert(mSN.end(), sn->parameters().begin(), sn->parameters().end());
			delete sn;
		}
	}

	received = sendATCmd("SL", {}, 100);
	if (received)
	{
		if(AtResponse* sn = dynamic_cast<AtResponse*> (received))
		{
			if (sn->cmd() == "SL") mSN.insert(mSN.end(), sn->parameters().begin(), sn->parameters().end());
			delete sn;
		}
	}

	if (mSN.size() == 8)
	{
		LOG(INFO) << "Device initialised, SN: " << snString();
		sendATCmd("CH", {0x0B});
		sendATCmd("ID", {0x12, 0x13});
	}
	else
	{
		LOG(ERROR) << "Device not initalised, SN length: " << mSN.size();
	}

}

} /* namespace CommNs */
