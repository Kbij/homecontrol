/*
 * Serial.cpp
 *
 *  Created on: May 29, 2016
 *      Author: koen
 */

#include <Comm/Serial.h>
#include <Comm/SerialListenerIf.h>
#include <glog/logging.h>

namespace CommNs {

Serial::Serial(std::string port, unsigned int baudRate):
	mListener(nullptr),
	mIo(),
	mSerial(mIo,port),
	mSerialThreadRunning(false),
	mSerialThread(nullptr)
{
	mSerial.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
	mSerial.set_option(boost::asio::serial_port::flow_control( boost::asio::serial_port::flow_control::hardware));
	startSerialThread();
}

Serial::~Serial()
{
	stopSerialThread();
}

void Serial::registerSerialListener(SerialListenerIf* listener)
{
	mListener = listener;
}

void Serial::unRegisterSerialListener()
{
	mListener = nullptr;
}
/**
 * Write a string to the serial device.
 * \param s string to write
 * \throws boost::system::system_error on failure
 */
void Serial::writeLine(const std::string& line)
{
    boost::asio::write(mSerial,boost::asio::buffer(line.c_str(),line.size()));
}

void Serial::writeData(const std::vector<uint8_t>& data)
{
    boost::asio::write(mSerial,boost::asio::buffer(data,data.size()));
}
/**
 * Blocks until a line is received from the serial device.
 * Eventual '\n' or '\r\n' characters at the end of the string are removed.
 * \return a string containing the received line
 * \throws boost::system::system_error on failure
 */
std::string Serial::readLine()
{
    //Reading data char by char, code is optimized for simplicity, not speed
    using namespace boost;
    char c;
    std::string result;
    for(;;)
    {
        asio::read(mSerial,asio::buffer(&c,1));
        switch(c)
        {
            case '\r':
                break;
            case '\n':
                return result;
            default:
                result+=c;
        }
    }
}

std::vector<uint8_t> Serial::readData()
{
	LOG(INFO) << "reading data...";
    //Reading data char by char, code is optimized for simplicity, not speed
    std::vector<uint8_t> buffer(1);
//    buffer.reserve(100);
    std::string result;
    size_t bytesRead = boost::asio::read(mSerial,boost::asio::buffer(&buffer,1));
    LOG(INFO) << "Serial received, size: " << buffer.size();
    return buffer;
}

void Serial::startSerialThread()
{
	if (!mSerialThreadRunning)
	{
		mSerialThreadRunning = true;
		mSerialThread = new std::thread(&Serial::serialThread, this);
	}
}

void Serial::stopSerialThread()
{
	mSerialThreadRunning = false;
	mSerial.close();
    if (mSerialThreadRunning)
    {
    	mSerialThread->join();
    	delete mSerialThread;
    	mSerialThread = nullptr;
    }
}

void Serial::serialThread()
{
	while(mSerialThreadRunning)
	{
		std::vector<uint8_t> data = readData();
		if (data.size() > 0)
		{
			if (mListener)
			{
				mListener->receiveData(data);
			}
		}
	}
	LOG(INFO) << "Stop serial thread";
}

} /* namespace CommNs */
