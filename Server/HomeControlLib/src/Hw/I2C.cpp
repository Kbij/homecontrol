/*
 * I2C.cpp
 *
 *  Created on: May 12, 2018
 *      Author: koen
 */

#include <Hw/I2C.h>
#include <glog/logging.h>
#include <stdio.h>
#include <exception>
#include <iomanip>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

namespace HwNs {

I2C::I2C(const std::string& i2cFileName):
	mI2CFileName(i2cFileName),
	mBusMutex()
{
	int i2cFile;
	// Open port for reading and writing
	if ((i2cFile = open(mI2CFileName.c_str(), O_RDWR)) < 0)
	{
		std::string  ex("Failed to open bus (" + mI2CFileName + "): ");
		ex += strerror(errno);
		throw std::runtime_error(ex);
	}

	close(i2cFile);
}

I2C::~I2C()
{
}

bool I2C::writeData(uint8_t address, const std::vector<uint8_t>& data)
{
	std::vector<uint8_t> readData;
	return readWriteData(address, data, readData);
}

bool I2C::readWriteData(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData)
{
    std::lock_guard<std::mutex> lg(mBusMutex);

	if (writeData.size() == 0)
	{
		LOG(ERROR) << "Writing data with 0 size (address: " << (int) address << ")";
		return false;
	}
	std::ostringstream writeLogStream;
	writeLogStream << std::hex << std::setfill('0') << std::setw(2);
	bool writeFirst = true;
	for (auto byte: writeData)
	{
		if (!writeFirst)
		{
			writeLogStream << ", ";
		}
		writeFirst = false;

		writeLogStream << " 0x" << (int) byte;
	}
	VLOG(3) << "Writing I2C (address: " << (int) address << "): Data:" << writeLogStream.str() << ";";

	int i2cFile;
	if ((i2cFile = open(mI2CFileName.c_str(), O_RDWR)) < 0)
	{
		std::string  ex("Failed to open bus");
		ex += strerror(errno);
		throw ex;
	}
	// Set the port options and set the address of the device we wish to speak to
	if (ioctl(i2cFile, I2C_SLAVE, address) < 0)
	{
		close(i2cFile);
		return false;
	}

	if (writeData.size() > 0)
	{
		if ((write(i2cFile, writeData.data(), writeData.size())) != static_cast<int>(writeData.size()) )
		{
			close(i2cFile);
			return false;
		}
	}

	if (readData.size() > 0)
	{
		if (read(i2cFile, readData.data(), readData.size()) != static_cast<int>(readData.size()))
		{
			LOG(ERROR) << "Failed reading data (address: " << (int) address << "): " << strerror(errno);

			close(i2cFile);
			return false;
		}

		std::ostringstream writeLogStream;
		writeLogStream << std::hex << std::setfill('0') << std::setw(2);

		bool readFirst = true;
		for (auto byte: readData)
		{
			if (!readFirst)
			{
				writeLogStream << ", ";
			}
			readFirst = false;

			writeLogStream << " 0x" << (int) byte;
		}
		VLOG(3) << "Data read (address: " << (int) address << "):" << writeLogStream.str() << ";";
	}

	close(i2cFile);
	return true;
}


} /* namespace HwNs */
