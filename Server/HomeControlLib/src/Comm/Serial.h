/*
 * Serial.h
 *
 *  Created on: May 29, 2016
 *      Author: koen
 */

#ifndef COMM_SERIAL_H_
#define COMM_SERIAL_H_
#include <string>
#include <boost/asio.hpp>

namespace CommNs {

class Serial {
public:
	Serial(std::string port, unsigned int baudRate);
	virtual ~Serial();

	void writeString(std::string s);
	std::string readLine();
private:
    boost::asio::io_service mIo;
    boost::asio::serial_port mSerial;
};

} /* namespace CommNs */

#endif /* COMM_SERIAL_H_ */
