/*
 * Serial.cpp
 *
 *  Created on: May 29, 2016
 *      Author: koen
 */

#include <Comm/Serial.h>
#include <Comm/SerialListenerIf.h>

namespace CommNs {

Serial::Serial(std::string port, unsigned int baudRate):
	mListener(nullptr),
	mIo(),
	mSerial(mIo,port),
	mSerialThreadRunning(false),
	mSerialThread(nullptr)
{
	mSerial.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
}

Serial::~Serial()
{
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
		std::string line = readLine();
		if (line.size() > 0)
		{
			if (mListener)
			{
				mListener->receiveLine(line);
			}
		}

	}
}

} /* namespace CommNs */
