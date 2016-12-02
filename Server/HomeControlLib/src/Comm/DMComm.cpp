/*
 * DMComm.cpp
 *
 *  Created on: Jul 9, 2016
 *      Author: koen
 */

#include <Comm/DMComm.h>
#include "Comm/DMFrameProcessorIf.h"
#include "Comm/DMMessages.h"
#include "Comm/DMCommListenerIf.h"
#include "Comm/CommUtils.h"
#include "glog/logging.h"
#include <iomanip>
#include <sstream>
#include <iostream>

namespace CommNs {

DMComm::DMComm(DMFrameProcessorIf* frameProcessor, uint8_t channel, std::vector<uint8_t> id):
	mFrameProcessor(frameProcessor),
	mChannel(channel),
	mId(id),
	mListener(nullptr),
	mFrameId(1),
	mSynchronousFrameId(0),
    mWaitForResponseCondVar(),
    mResponseReceived(),
	mConditionVarMutex(),
	mSN(),
	mDataMutex()
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

void DMComm::registerListener(DMCommListenerIf* listener)
{
	std::lock_guard<std::recursive_mutex> lg(mDataMutex);
	mListener = listener;
}

void DMComm::unRegisterListener(DMCommListenerIf* /*listener*/)
{
	std::lock_guard<std::recursive_mutex> lg(mDataMutex);
	mListener = nullptr;
}

std::string DMComm::addressString()
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

DMMessageIf* DMComm::sendMessage(DMMessageIf* message, int timeOutMilliseconds)
{
	mResponseReceived = false;
	mSynchronousFrameId = mFrameId;
	mReceivedMessage = nullptr;
	std::string sendMessageDetail = message->toString();

	//Message is deleted here ....
	sendMessage(message);
	auto until = std::chrono::system_clock::now() +std::chrono::milliseconds(timeOutMilliseconds);
	std::unique_lock<std::mutex> lock(mConditionVarMutex);
	mWaitForResponseCondVar.wait_until(lock, until,[&]{return (bool)mResponseReceived;});
	if (mResponseReceived)
	{
		VLOG(10) << "Response received for message: " << sendMessageDetail;
		return mReceivedMessage;
	}
	else
	{
		LOG(ERROR) << "No response received for message: " << sendMessageDetail << ", timeout: " << timeOutMilliseconds;
	}
	mSynchronousFrameId = 0;

	return nullptr;
}

void DMComm::receiveFrame(const std::vector<uint8_t>& data)
{
	if (VLOG_IS_ON(11))
	{
		printFrame(data);
	}

	if (data.size() < 1) return;
	bool synchronousResponse = false;
	const uint8_t frameType = data[0];
	const uint8_t frameId = data[1];
	if (mSynchronousFrameId == frameId && frameId != 0x00)
	{
		synchronousResponse = true;
	}

	DMMessageIf* received = nullptr;

	switch(frameType)
	{
		case 0x88:
		{
			std::string atCmd(data.begin() + 2, data.begin() + 4);
			received = new AtResponse(atCmd, std::vector<uint8_t>(data.begin() + 5, data.end()));
			break;
		}
		case 0x8A:
		{
			received = new ModemStatusFrame(std::vector<uint8_t>(data.begin(), data.end()));
			break;
		}
		case 0x8B:
		{
			received = new TxStatusFrame(std::vector<uint8_t>(data.begin(), data.end()));
			break;
		}
		case 0x90:
		{
			received = new RxMessage(std::vector<uint8_t>(data.begin(), data.end()));
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
		VLOG(10) << "Synchronous reponse";
		mResponseReceived = true;
		mReceivedMessage = received;
		mWaitForResponseCondVar.notify_one();
	}
	else
	{
		std::lock_guard<std::recursive_mutex> lg(mDataMutex);
		if (received)
		{
			if (mListener)
			{
				mListener->receiveMessage(received);
			}
			delete received;
		}
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
	VLOG(11) << "Received frame: " << ss.str();
}

void DMComm::init()
{
	mFrameId = 1;
	DMMessageIf* received = sendATCmd("A2", {0x06}, 100);
	if (received)
	{
		delete received;
	}

	received = sendATCmd("NI", {'S', 'e', 'r', 'v', 'e', 'r'}, 100);
	if (received)
	{
		delete received;
	}

	received = sendATCmd("SH", {}, 100);
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
		LOG(INFO) << "Device initialised, Address: " << addressString();
		sendATCmd("CH", {mChannel});
		sendATCmd("ID", mId);
	}
	else
	{
		LOG(ERROR) << "Device not initalised, SN length: " << mSN.size();
	}

}

} /* namespace CommNs */
