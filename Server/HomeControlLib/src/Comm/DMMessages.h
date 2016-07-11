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

enum class DMMessageType {ATResponse_SH, ATResponse_SL};

class DMMessageIf
{
public:
	virtual ~DMMessageIf() {};

	virtual DMMessageType messageType() = 0;
	virtual std::string toString() = 0;
	virtual std::vector<uint8_t> serialise() = 0;

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
	std::vector<uint8_t> serialise() {return std::vector<uint8_t>();};
	DMMessageType mMessageType;
	std::vector<uint8_t> SN;
};

} /* namespace CommNs */


#endif /* COMM_DMMESSAGES_H_ */
