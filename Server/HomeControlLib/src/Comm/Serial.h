/*
 * Serial.h
 *
 *  Created on: May 29, 2016
 *      Author: koen
 */

#ifndef COMM_SERIAL_H_
#define COMM_SERIAL_H_
#include "SerialIf.h"
#include <string>
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <stdint.h>

namespace CommNs {
class SerialListenerIf;

class Serial: public SerialIf
{
public:
	Serial(std::string port, unsigned int baudRate);
	virtual ~Serial();

	void registerSerialListener(SerialListenerIf* listener);
	void unRegisterSerialListener();

	void writeLine(const std::string& line);
	void writeData(const std::vector<uint8_t>& data);

private:
	SerialListenerIf* mListener;
    boost::asio::io_service mIo;
    boost::asio::serial_port mSerial;
	bool mSerialThreadRunning;
	std::thread* mSerialThread;

	std::string readLine();
	std::vector<uint8_t> readData();
	void startSerialThread();
	void stopSerialThread();
	void serialThread();
};

} /* namespace CommNs */

#endif /* COMM_SERIAL_H_ */
