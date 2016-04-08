/*
 * ObjectWriter.h
 *
 *  Created on: Apr 8, 2016
 *      Author: koen
 */

#ifndef DAL_OBJECTWRITER_H_
#define DAL_OBJECTWRITER_H_
#include "Comm/CommListenerIf.h"


namespace CommNs
{
class CommObjectIf;
}
namespace DalNs {

class ObjectWriter: public CommNs::CommListenerIf
{
public:
	ObjectWriter();
	virtual ~ObjectWriter();
	void receiveObject(const std::string name, const CommNs::CommObjectIf* object);
};

} /* namespace DalNs */

#endif /* DAL_OBJECTWRITER_H_ */
