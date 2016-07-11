/*
 * DMCommListener.h
 *
 *  Created on: Jul 9, 2016
 *      Author: koen
 */

#ifndef COMM_DMCOMMLISTENER_H_
#define COMM_DMCOMMLISTENER_H_


namespace CommNs {

class DMCommListenerIf
{
public:
	virtual ~DMCommListenerIf() {};

	virtual void receiveMessage(const DMMessageIf* message) = 0;
};

} /* namespace CommNs */


#endif /* COMM_DMCOMMLISTENER_H_ */
