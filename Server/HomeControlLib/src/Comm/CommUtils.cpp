/*
 * CommUtils.cpp
 *
 *  Created on: Aug 1, 2016
 *      Author: koen
 */

#include "CommUtils.h"
#include <iomanip>
#include <sstream>

namespace CommNs
{
std::string toReadableString(const std::vector<uint8_t>& data, bool printAcii)
{
	std::stringstream ss;
	bool first = true;
	bool prevHex = false;
	for(auto val: data)
	{
		if (!first && prevHex)
		{
			ss << ", ";
		}
		if ((val >= 32 && val < 128) && printAcii)
		{
			ss << val;
			prevHex = false;
		}
		else
		{
			ss << "0x" << std::hex << std::uppercase <<  std::setfill('0') << std::setw(2) <<  (int) val;
			prevHex = true;
		}
		first = false;
	}

	return ss.str();
}
}
