/*
 * DMMessages.h
 *
 *  Created on: Jul 9, 2016
 *      Author: koen
 */

#ifndef COMM_DMMESSAGES_H_
#define COMM_DMMESSAGES_H_
#include <vector>
#include <stdint.h>
#include <sstream>
#include <iomanip>

namespace CommNs {

enum class DMMessageType {ATResponse_SH, ATResponse_SL, TxRequestFrame, TxStatusFrame};

class DMMessageIf
{
public:
	virtual ~DMMessageIf() {};

	virtual DMMessageType messageType() = 0;
	virtual std::string toString() = 0;
	virtual std::vector<uint8_t> serialise(uint8_t frameId) = 0;

};

class AtResponse_SN: public DMMessageIf
{
public:
	AtResponse_SN(DMMessageType messageType) :
		mMessageType(messageType),
		SN()
	{
	}
	virtual ~AtResponse_SN() {};

	DMMessageType messageType() {return mMessageType;};
	std::string snString()
	{
		std::stringstream ss;
		bool first = true;
		for(auto val: SN)
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

	std::string toString()
	{
		return "SN: " + snString();
	}
	std::vector<uint8_t> serialise(uint8_t frameId) {return std::vector<uint8_t>();};
	DMMessageType mMessageType;
	std::vector<uint8_t> SN;
};

class TxMessage: public DMMessageIf
{
public:
	TxMessage(std::vector<uint8_t> txData, std::vector<uint8_t> destination) :
		mTxData(txData),
		mDestination(destination)
	{
	}
	virtual ~TxMessage() {};

	DMMessageType messageType() {return DMMessageType::TxRequestFrame;};

	std::string toString()
	{
		return "Tx Data, data size: " + mTxData.size();
	}
	std::vector<uint8_t> serialise(uint8_t frameId)
	{
		std::vector<uint8_t> data;
		data.push_back(0x10);
		data.push_back(frameId);
		data.insert(data.end(), mDestination.begin(), mDestination.end());
		data.push_back(0xFF);  // Reserved
		data.push_back(0xFE);  // Reserved
		data.push_back(0x00);  // Broadcast radius
		data.push_back(0x00);  // Transmit options
		data.insert(data.end(), mTxData.begin(), mTxData.end());

		return data;
	};

	std::vector<uint8_t> mTxData;
	std::vector<uint8_t> mDestination;
};
} /* namespace CommNs */


#endif /* COMM_DMMESSAGES_H_ */
