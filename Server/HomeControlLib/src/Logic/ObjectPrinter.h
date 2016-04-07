/*
 * ObjectPrinter.h
 *
 *  Created on: Apr 7, 2016
 *      Author: koen
 */

#ifndef LOGIC_OBJECTPRINTER_H_
#define LOGIC_OBJECTPRINTER_H_
#include "Comm/CommListenerIf.h"

namespace CommNs {

class ObjectPrinter: public CommListenerIf
{
public:
	ObjectPrinter();
	virtual ~ObjectPrinter();
	void receiveObject(const std::string name, const CommObjectIf& object);
};

} /* namespace CommNs */

#endif /* LOGIC_OBJECTPRINTER_H_ */
