/*
 * GpsLocation.h
 *
 *  Created on: Apr 4, 2016
 *      Author: koen
 */

#ifndef COMM_GPSLOCATION_H_
#define COMM_GPSLOCATION_H_
#include "CommObjectIf.h"
#include <string>
#include <ctime>

namespace CommNs {

class GpsLocation: public CommObjectIf
{
public:
	GpsLocation(const std::string& json);
	virtual ~GpsLocation();

	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const {return "";};

	double latitude() const {return mLatitude;};
	double longitude() const {return mLongitude;};
	double accuracy() const {return mAccuracy;};
	int batteryLevel() const {return mBatteryLevel;};
	time_t timeStamp() const {return mTimeStamp;};
private:
	double mLatitude;
	double mLongitude;
	double mAccuracy;
	int mBatteryLevel;
	std::time_t mTimeStamp;
};

} /* namespace CommNs */

#endif /* COMM_GPSLOCATION_H_ */
