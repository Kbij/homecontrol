/*
 * MainTest.cpp
 *
 *  Created on: Mar 28, 2016
 *      Author: koen
 */

#include "gtest/gtest.h"
#include <glog/logging.h>
#include <gflags/gflags.h>

GTEST_API_ int main(int argc, char **argv)
{
	google::SetVersionString("1.0.0");
	google::ParseCommandLineFlags(&argc, &argv, true);

	google::InitGoogleLogging("Home Control Test");
	testing::InitGoogleTest(&argc, argv);
//	testing::GTEST_FLAG(filter) = "-LiveTest.*";

//	testing::GTEST_FLAG(filter) = "LiveTest.SendApiCommand";
	testing::GTEST_FLAG(filter) = "DMTests.*";

	FLAGS_logtostderr=1;
	FLAGS_v = 3;

	//FLAGS_minloglevel = 100;
    int result = RUN_ALL_TESTS();

	google::ShutdownGoogleLogging();
	google::ShutDownCommandLineFlags();
	return result;
}
