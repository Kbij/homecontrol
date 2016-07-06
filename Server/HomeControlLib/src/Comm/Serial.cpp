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
	mPort(nullptr),
	mReadBuffer(),
	mMutex()
{

}

Serial::~Serial()
{
	stop();
}

void Serial::registerSerialListener(SerialListenerIf* listener)
{
	mListener = listener;
}

void Serial::unRegisterSerialListener()
{
	mListener = nullptr;
}

void Serial::writeLine(const std::string& line)
{
 //   boost::asio::write(mSerial,boost::asio::buffer(line.c_str(),line.size()));
}

void Serial::writeData(const std::vector<uint8_t>& data)
{
	boost::system::error_code ec;

	if (!mPort) return ;
	if (data.size() == 0) return;
	LOG(INFO) << "Writing data, size: " << data.size();
	mPort->write_some(boost::asio::buffer(data, data.size()), ec);
}

bool Serial::start(const std::string& portName, int baudRate)
{
	boost::system::error_code ec;

	if (mPort)
	{
		LOG(ERROR) << "error : port is already opened...";
		return false;
	}

	mPort = boost::shared_ptr<boost::asio::serial_port>(new boost::asio::serial_port(mIo));
	mPort->open(portName.c_str(), ec);
	if (ec)
	{
		LOG(ERROR ) << "error: open() failed...com_port_name= " << portName << ", error=" << ec.message();
		return false;
	}

	// option settings...
	mPort->set_option(boost::asio::serial_port_base::baud_rate(baudRate));
	mPort->set_option(boost::asio::serial_port_base::character_size(8));
	mPort->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
	mPort->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
	mPort->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

	boost::thread t(boost::bind(&boost::asio::io_service::run, &mIo));

	asyncReadSome();

	return true;
}

void Serial::stop()
{
	boost::mutex::scoped_lock look(mMutex);

	if (mPort)
	{
		mPort->cancel();
		mPort->close();
		mPort.reset();
	}
	mIo.stop();
	mIo.reset();
}

void Serial::asyncReadSome()
{
	if (mPort.get() == NULL || !mPort->is_open()) return;

	VLOG(1) << "Async read";
	mPort->async_read_some(boost::asio::buffer(mReadBuffer, SERIAL_PORT_READ_BUF_SIZE),
						   boost::bind(&Serial::onReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Serial::onReceive(const boost::system::error_code& ec, size_t bytesTransferred)
{
	VLOG(1) << "onReceive";
	boost::mutex::scoped_lock look(mMutex);

	if (mPort.get() == NULL || !mPort->is_open()) return;
	if (ec)
	{
		asyncReadSome();
		return;
	}
	VLOG(1) << "Bytes received: " << bytesTransferred;

//	for (unsigned int i = 0; i < bytes_transferred; ++i) {
//		char c = read_buf_raw_[i];
//		if (c == end_of_line_char_) {
//			this->on_receive_(read_buf_str_);
//			read_buf_str_.clear();
//		}
//		else {
//			read_buf_str_ += c;
//		}
//	}
//
//async_read_some_();
}
} /* namespace CommNs */
