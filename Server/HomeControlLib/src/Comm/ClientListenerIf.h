/*
 * ClientListenerIf.h
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#ifndef COMM_CLIENTLISTENERIF_H_
#define COMM_CLIENTLISTENERIF_H_
#include <string>

namespace CommNs {
class CommObjectIf;
class Client;

class ClientListenerIf
{
public:
	virtual ~ClientListenerIf() {};

	virtual Client* newClient() = 0;
	virtual void receiveObject(const std::string name, const CommObjectIf& object) = 0;
};

} /* namespace CommNs */


#endif /* COMM_CLIENTLISTENERIF_H_ */
