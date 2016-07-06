/*
 * XbeeTest.cpp
 *
 *  Created on: Jul 2, 2016
 *      Author: koen
 */
#include <stdint.h>
#include "Comm/Serial.h"
//#define POSIX
//#include "xbee/platform.h"
//#include "xbee/byteorder.h"
//#include "xbee/device.h"
//#include "xbee/atcmd.h"
//#include "wpan/types.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"

DEFINE_string(serial, "/dev/ttyUSB0", "Serial port to use");

//const xbee_dispatch_table_entry_t xbee_frame_handlers[] =
//{
//		XBEE_FRAME_HANDLE_LOCAL_AT,
//		XBEE_FRAME_MODEM_STATUS_DEBUG,
//		XBEE_FRAME_TABLE_END
//};

//TEST(Xbee, Init)
//{
//	xbee_dev_t my_xbee;
//	xbee_serial_t XBEE_SERPORT = { 115200, 0 , "/tmp/ttyUSB0"};
//	int status;
//
//	// initialize the serial and device layer for this XBee device
//	if (xbee_dev_init( &my_xbee, &XBEE_SERPORT, NULL, NULL))
//	{
//		LOG(INFO) << "Failed to initialize device.";
//	}
//
//	xbee_cmd_init_device( &my_xbee);
//	LOG(INFO) << "Waiting for driver to query the XBee device...";
//	do {
//		xbee_dev_tick( &my_xbee);
////		xbee_dev_tick( &my_xbee);
////		xbee_dev_tick( &my_xbee);
//		status = xbee_cmd_query_status( &my_xbee);
//	} while (status == -EBUSY);
//	if (status)
//	{
//		LOG(ERROR) << "Error " << status << "  waiting for query to complete.";
//	}
//}
TEST(Xbee, Serial)
{
	LOG(INFO) << "Serial port: " << FLAGS_serial;
	CommNs::Serial serial(FLAGS_serial, 115200);
	serial.start(FLAGS_serial, 38400);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	//std::vector<uint8_t> testData {0x7E, 0x00, 0x04, 0x08, 0x01, 0x61, 0x74, 0x21};
	std::vector<uint8_t> testData {'+', '+', '+'};
	serial.writeData(testData);
	std::this_thread::sleep_for(std::chrono::seconds(3));

	std::vector<uint8_t> testVL {'A', 'T', 'V', 'L', '\n'};
	serial.writeData(testVL);
	std::this_thread::sleep_for(std::chrono::seconds(1));

}
