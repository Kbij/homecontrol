/*
** EPITECH PROJECT, 2025
** homecontrol (Workspace)
** File description:
** RestService
*/

#include "RestService.h"
#include "DAL/HomeControlDalIf.h"
#include <glog/logging.h>

namespace RestServerNs
{
RestService::RestService(DalNs::HomeControlDalIf* homeControlDal):
    mHomeControlDal(homeControlDal),
    mDevices()
{
}

RestService::~RestService()
{
}

time_t iso8601ToEpoch(const std::string& iso8601)
{
    time_t result = 0;
    struct tm ctime;
    memset(&ctime, 0, sizeof(struct tm));
    strptime(iso8601.c_str(), "%FT%T%z", &ctime);

    result = timegm(&ctime);
    return result;
}

void RestService::logPosition(const std::string& device, const std::string& timestampStr, const std::string& latStr, const std::string& lonStr, const std::string& battStr, const std::string& accuracyStr)
{
    if (mHomeControlDal)
    {
        LOG(INFO) << "Device: " << device << ", time: " << timestampStr << ", lat: " << latStr << ", lon: " << lonStr << ", batt: " << battStr << ", accuracy: " << accuracyStr;

        //First time we see the devcee
        if (mDevices.find(device) == mDevices.end())
        {
            auto deviceId = mHomeControlDal->findDevice(device);
            if (deviceId >= 0)
            {
                mDevices[device] = deviceId;
            }
        }

        if (mDevices.find(device) != mDevices.end())
        {
            double lat = std::stod(latStr);
            double lon = std::stod(lonStr);
            double batt = std::stod(battStr);
            double accuracy = std::stod(accuracyStr);
            time_t time = iso8601ToEpoch(timestampStr);
        //2025-01-21T20:37:17.386Z
//timestampStr

            mHomeControlDal->logLocation(mDevices[device], lat, lon, accuracy, batt, time);

        }
    }
}
}