/*
 * ObjectPrinter.h
 *
 *  Created on: Apr 7, 2016
 *      Author: koen
 */

#ifndef LOGIC_OBJECTPRINTER_H_
#define LOGIC_OBJECTPRINTER_H_
#include "Comm/CommListenerIf.h"

namespace LogicNs {

class ObjectPrinter: public CommNs::CommListenerIf
{
public:
	ObjectPrinter();
	virtual ~ObjectPrinter();

	void clientConnected(const std::string& name);
	void clientDisConnected(const std::string& name);
	void receiveObject(const std::string name, const CommNs::CommObjectIf* object);
};

} /* namespace LogicNs */

#endif /* LOGIC_OBJECTPRINTER_H_ */
