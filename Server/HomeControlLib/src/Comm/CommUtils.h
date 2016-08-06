/*
 * CommUtils.h
 *
 *  Created on: Aug 1, 2016
 *      Author: koen
 */

#ifndef COMM_COMMUTILS_H_
#define COMM_COMMUTILS_H_
#include <vector>
#include <stdint.h>
#include <string>

namespace CommNs
{
std::string toReadableString(const std::vector<uint8_t>& data, bool printAcii);
}

#endif /* COMM_COMMUTILS_H_ */
