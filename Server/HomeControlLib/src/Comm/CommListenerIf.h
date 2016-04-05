/*
 * CommListenerIf.h
 *
 *  Created on: Apr 4, 2016
 *      Author: koen
 */

#ifndef COMM_COMMLISTENERIF_H_
#define COMM_COMMLISTENERIF_H_


namespace CommNs {
class CommObjectIf;

class CommListenerIf
{
public:
	virtual ~CommListenerIf() {};

	virtual void receiveObject(const std::string name, const CommObjectIf& object) = 0;
};

} /* namespace CommNs */


#endif /* COMM_COMMLISTENERIF_H_ */
