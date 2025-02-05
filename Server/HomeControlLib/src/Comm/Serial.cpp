/*
 * Serial.cpp
 *
 *  Created on: May 29, 2016
 *      Author: koen
 */

#include <Comm/Serial.h>
#include <Comm/SerialListenerIf.h>
#include <glog/logging.h>
#include <boost/bind.hpp>
#include <iostream>

namespace CommNs {

Serial::Serial(const std::string& portName, unsigned int baudRate):
	mPortName(portName),
	mBaudRate(baudRate),
	mListener(nullptr),
	mIo(),
	mPort(nullptr),
	mReadBuffer(100),
	mThread(nullptr),
	mMutex()
{
}

Serial::~Serial()
{
	unRegisterSerialListener();
	closeSerial();
}

void Serial::registerSerialListener(SerialListenerIf* listener)
{
	mListener = listener;
}

void Serial::unRegisterSerialListener()
{
	mListener = nullptr;
}

void Serial::writeData(const std::vector<uint8_t>& data)
{
	boost::system::error_code ec;

	if (!mPort) return ;
	if (data.size() == 0) return;
	VLOG(30) << "Writing data, size: " << data.size();
	mPort->write_some(boost::asio::buffer(data, data.size()), ec);
}

bool Serial::openSerial()
{
	try
	{
		boost::system::error_code ec;
		LOG(INFO) << "Opening port: " << mPortName << ", baudrate: " << mBaudRate;

		if (mPort)
		{
			LOG(ERROR) << "Port is already opened...";
			return false;
		}

		mPort = boost::shared_ptr<boost::asio::serial_port>(new boost::asio::serial_port(mIo));
		mPort->open(mPortName.c_str(), ec);
		if (ec)
		{
			LOG(ERROR ) << "OpenSerial failed... port: " << mPortName << ", error:" << ec.message();
			return false;
		}

		// option settings...
		mPort->set_option(boost::asio::serial_port_base::baud_rate(mBaudRate));
		mPort->set_option(boost::asio::serial_port_base::character_size(8));
		mPort->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
		mPort->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
		mPort->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

		asyncReadSome();
		mThread = new std::thread(&Serial::serialThread, this);
	}
	catch(...)
	{
		LOG(ERROR) << "Exception opening serial port";
	}
	return true;
}

void Serial::closeSerial()
{
	{
		std::lock_guard<std::mutex> lock(mMutex);
		VLOG(30) << "Stop serial";
		if (mPort && mPort->is_open())
		{
			mPort->cancel();
			mPort->close();
		}
	}
	mIo.stop();
	//mIo.reset();
	//mPort.reset();
	if (mThread)
	{
		mThread->join();
		delete mThread;
	}
}

void Serial::serialThread()
{
	mIo.run();
}

void Serial::asyncReadSome()
{
	if (!mPort || !mPort->is_open()) return;

	mPort->async_read_some(boost::asio::buffer(mReadBuffer, SERIAL_PORT_READ_BUF_SIZE),
						   boost::bind(&Serial::onReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Serial::onReceive(const boost::system::error_code& ec, size_t bytesTransferred)
{
	VLOG(30) << "Bytes received: " << bytesTransferred;
	std::lock_guard<std::mutex> lock(mMutex);
	if (!mPort) return;
	if (!mPort->is_open()) return;
	if (ec)
	{
		LOG(ERROR) << "Receive error: " << ec;
	}

	if (mListener)
	{
		mListener->receiveData(std::vector<uint8_t>(mReadBuffer.begin(), mReadBuffer.begin() + bytesTransferred));
	}
	asyncReadSome();
}
} /* namespace CommNs */
