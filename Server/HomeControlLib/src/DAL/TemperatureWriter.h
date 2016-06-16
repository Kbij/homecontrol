/*
 * TemperatureWriter.h
 *
 *  Created on: May 31, 2016
 *      Author: koen
 */

#ifndef DAL_TEMPERATUREWRITER_H_
#define DAL_TEMPERATUREWRITER_H_
#include "Logic/TemperatureListenerIf.h"
namespace CommNs
{
class TemperatureSourceIf;
}

namespace DalNs {

class TemperatureWriter: public LogicNs::TemperatureListenerIf
{
public:
	TemperatureWriter(CommNs::TemperatureSourceIf* sensors);
	virtual ~TemperatureWriter();

	void sensorStarted(const std::string& sensorId);
	void sensorTemperature(const std::string& sensorId, double temperature);
	void sensorSetTemperatureUp(const std::string& sensorId);
	void sensorSetTemperatureDown(const std::string& sensorId);
private:
	CommNs::TemperatureSourceIf* mSensors;
};

} /* namespace DalNs */

#endif /* DAL_TEMPERATUREWRITER_H_ */
