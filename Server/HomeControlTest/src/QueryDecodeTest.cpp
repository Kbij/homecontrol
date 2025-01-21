/*
** EPITECH PROJECT, 2025
** homecontrol (Workspace)
** File description:
** QueryDecodeTest
*/

#include "Rest/QueryDecode.h"
#include <gtest/gtest.h>
#include <glog/stl_logging.h>
#include <glog/logging.h>

TEST(QueryDecodeTest, Simple)
{
    auto params = RestServerNs::queryDecode("lat=51.0535538&longitude=3.6441198&time=2025-01-20T21:13:12.128Z&s=0.0&AID=853b1e43728bf48b&BATT=58.0");
    EXPECT_EQ((size_t) 6, params.size());

    EXPECT_EQ("51.0535538", params["lat"]);
    EXPECT_EQ("3.6441198", params["longitude"]);
    EXPECT_EQ("2025-01-20T21:13:12.128Z", params["time"]);
    EXPECT_EQ("853b1e43728bf48b", params["AID"]);
    EXPECT_EQ("58.0", params["BATT"]);
}
