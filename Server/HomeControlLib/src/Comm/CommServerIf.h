/*
 * CommServerIf.h
 *
 *  Created on: Jun 7, 2016
 *      Author: koen
 */

#ifndef COMM_COMMSERVERIF_H_
#define COMM_COMMSERVERIF_H_
#include <string>

namespace CommNs {
class CommListenerIf;
class CommObjectIf;

class CommServerIf
{
public:
	virtual ~CommServerIf() {};

	virtual void registerCommListener(CommListenerIf* listener) = 0;
	virtual void unRegisterCommListener(CommListenerIf* listener) = 0;
	// Server takes ownership
	virtual void sendObject(const std::string name, CommObjectIf* object) = 0;
};

} /* namespace CommNs */



#endif /* COMM_COMMSERVERIF_H_ */
