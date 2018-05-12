/*
 * I2CIf.h
 *
 *  Created on: May 12, 2018
 *      Author: koen
 */

#ifndef HW_I2CIF_H_
#define HW_I2CIF_H_
#include <stdint.h>
#include <vector>

namespace HwNs {

class I2CIf {
public:

	virtual bool writeData(uint8_t address, const std::vector<uint8_t>& data) = 0;

};

} /* namespace HwNs */




#endif /* HW_I2CIF_H_ */
