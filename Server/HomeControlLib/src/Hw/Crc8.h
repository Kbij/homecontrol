/*
 * Crc8.h
 *
 *  Created on: May 13, 2018
 *      Author: koen
 */

#ifndef HW_CRC8_H_
#define HW_CRC8_H_
#include <stdint.h>
#include <vector>

inline uint8_t Crc8(const std::vector<uint8_t>& data)
{
	uint8_t crc = 0x00;
	for(auto extract: data)
	{
		for (uint8_t tempI = 8; tempI; tempI--)
		{
			uint8_t sum = (crc ^ extract) & 0x01;
			crc >>= 1;
			if (sum)
			{
				crc ^= 0x8C;
			}
			extract >>= 1;
		}
	}
	return crc;
}


#endif /* HW_CRC8_H_ */
