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
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
//#include <boost/bind.hpp>
//#include <boost/thread.hpp>
#include <thread>
#include <vector>
#include <stdint.h>
#include <mutex>

namespace
{
const int SERIAL_PORT_READ_BUF_SIZE  = 256;
}

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

	bool start(const std::string& portName, int baudRate);
	void stop();
private:
	SerialListenerIf* mListener;
    boost::asio::io_service mIo;
    boost::shared_ptr<boost::asio::serial_port> mPort;
    char mReadBuffer[SERIAL_PORT_READ_BUF_SIZE];
    std::thread* mThread;
    std::mutex mMutex;

	void asyncReadSome();
	void onReceive(const boost::system::error_code& ec, size_t bytesTransferred);
	void serialThread();
	//void on_receive_(const std::string &data);

};

} /* namespace CommNs */

#endif /* COMM_SERIAL_H_ */
