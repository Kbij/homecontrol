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

enum class DMMessageType {ATResponse, TxRequestFrame, TxStatusFrame};

class DMMessageIf
{
public:
	virtual ~DMMessageIf() {};

	virtual DMMessageType messageType() = 0;
	virtual std::string toString() = 0;
	virtual std::vector<uint8_t> serialise(uint8_t frameId) = 0;

};

class AtResponse: public DMMessageIf
{
public:
	AtResponse(std::string cmd, std::vector<uint8_t> parameters) :
		mCmd(cmd),
		mParameters(parameters)
	{
	}
	virtual ~AtResponse() {};

	DMMessageType messageType() {return DMMessageType::ATResponse;};

	std::string toString()
	{
		std::stringstream ss;
		ss << "AT Response, Cmd: " << mCmd << ", parameter length: " << mParameters.size();
		return ss.str();
	}
	std::vector<uint8_t> serialise(uint8_t frameId) {return std::vector<uint8_t>();};

	std::string cmd() {return mCmd;};
	std::vector<uint8_t>& parameters() {return mParameters;};
private:
	std::string mCmd;
	std::vector<uint8_t> mParameters;
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
		std::stringstream ss;
		ss << "Tx Data, data size: " << mTxData.size();
		return ss.str();
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

enum class DeliveryStatus {	Success = 0x00
							,MACACKFailure = 0x01
							,CollisionAvoidanceFailure = 0x02
							,NetworkACKFailure = 0x21
							,RouteNotFound = 0x25
							,InternalResourceError = 0x31
							,InternalError = 0x32
							,PayloadTooLarge = 0x74 };
enum class DiscoveryStatus { NoDiscoveryOverhead = 0x00
							 ,RouteDiscovery = 0x02};

class TxStatusFrame: public DMMessageIf
{
public:
	TxStatusFrame(std::vector<uint8_t> data):
		mTransmitRetry(),
		mDeliveryStatus(),
		mDiscoveryStatus()
	{
		mTransmitRetry = data[4];
		mDeliveryStatus = (DeliveryStatus) data[5];
		mDiscoveryStatus = (DiscoveryStatus) data[6];
	}
	virtual ~TxStatusFrame() {};

	DMMessageType messageType() {return DMMessageType::TxStatusFrame;};

	std::string toString()
	{
		std::stringstream ss;
		ss << "Tx Status. Transmit Retry: " << (int) mTransmitRetry << ", Delivery status: ";
		switch(mDeliveryStatus)
		{
			case DeliveryStatus::Success: ss << "Success"; break;
			case DeliveryStatus::MACACKFailure: ss << "Mac Ack Failure"; break;
			case DeliveryStatus::CollisionAvoidanceFailure: ss << "Mac Ack Failure"; break;
			case DeliveryStatus::NetworkACKFailure: ss << "Mac Ack Failure"; break;
			case DeliveryStatus::RouteNotFound: ss << "Mac Ack Failure"; break;
			case DeliveryStatus::InternalResourceError: ss << "Mac Ack Failure"; break;
			case DeliveryStatus::InternalError: ss << "Mac Ack Failure"; break;
			case DeliveryStatus::PayloadTooLarge: ss << "Mac Ack Failure"; break;
			default: ss << "Unknown";
		}
		ss << ", Discovery status: ";
		switch(mDiscoveryStatus)
		{
			case DiscoveryStatus::NoDiscoveryOverhead: ss << "No Discovery overhead"; break;
			case DiscoveryStatus::RouteDiscovery: ss << "Route discovery"; break;
		}

		return ss.str();
	}
	std::vector<uint8_t> serialise(uint8_t frameId) {return std::vector<uint8_t>();};
private:
	uint8_t mTransmitRetry;
	DeliveryStatus mDeliveryStatus;
	DiscoveryStatus mDiscoveryStatus;
};
} /* namespace CommNs */


#endif /* COMM_DMMESSAGES_H_ */
