/*
 * DMCommIf.h
 *
 *  Created on: Aug 2, 2016
 *      Author: koen
 */

#ifndef COMM_DMCOMMIF_H_
#define COMM_DMCOMMIF_H_
#include <string>
#include <vector>
#include <stdint.h>

namespace CommNs
{
class DMCommListenerIf;
class DMMessageIf;

class DMCommIf
{
public:
	virtual void registerListener(DMCommListenerIf* listener) = 0;
	virtual void unRegisterListener(DMCommListenerIf* listener) = 0;

	virtual void init() = 0;
	virtual std::string addressString() = 0;

	virtual void sendATCmd(const std::string& atCmd, std::vector<uint8_t> parameters) = 0;
	virtual DMMessageIf* sendATCmd(const std::string& atCmd, std::vector<uint8_t> parameters, int timeOutMilliseconds) = 0;
	virtual void sendMessage(DMMessageIf* message) = 0;
	virtual DMMessageIf* sendMessage(DMMessageIf* message, int timeOutMilliseconds) = 0;


};

} /* namespace CommNs */


#endif /* COMM_DMCOMMIF_H_ */
