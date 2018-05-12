/*
 * I2C.h
 *
 *  Created on: May 12, 2018
 *      Author: koen
 */

#ifndef HW_I2C_H_
#define HW_I2C_H_
#include <Hw/I2CIf.h>
#include <stdint.h>
#include <vector>
#include <mutex>
#include <string>


namespace HwNs
{

class I2C: public I2CIf
{
public:
	I2C(const std::string& i2cFileName);
	virtual ~I2C();

	bool writeData(uint8_t address, const std::vector<uint8_t>& data);
private:
	bool readWriteData(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData);

	std::string mI2CFileName;
	std::mutex mBusMutex;

};

} /* namespace HwNs */

#endif /* HW_I2C_H_ */
