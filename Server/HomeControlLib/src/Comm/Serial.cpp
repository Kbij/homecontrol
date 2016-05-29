/*
 * Serial.cpp
 *
 *  Created on: May 29, 2016
 *      Author: koen
 */

#include <Comm/Serial.h>

namespace CommNs {

Serial::Serial(std::string port, unsigned int baudRate):
		mIo(),
		mSerial(mIo,port)
{
	mSerial.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
}

Serial::~Serial()
{
}
/**
 * Write a string to the serial device.
 * \param s string to write
 * \throws boost::system::system_error on failure
 */
void Serial::writeString(std::string s)
{
    boost::asio::write(mSerial,boost::asio::buffer(s.c_str(),s.size()));
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
} /* namespace CommNs */
