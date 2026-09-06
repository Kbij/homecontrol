/*
 * AdminAuthRequest.h
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#ifndef COMMOBJECTS_ADMINAUTHREQUEST_H_
#define COMMOBJECTS_ADMINAUTHREQUEST_H_
#include "CommObjectIf.h"
#include <string>

namespace CommNs {

/**
 * Client -> server, objectId 40. Payload: JSON {"code": "<pin>"}. Sent once, right after
 * the user types the numeric admin PIN on the swipe-left screen. Kept as a string (not a
 * number) so a leading zero in the PIN survives the round trip.
 *
 * The PIN is only ever compared server-side (against Client.adminCode, see AdminController)
 * and is never sent back to any client.
 */
class AdminAuthRequest: public CommObjectIf
{
public:
	AdminAuthRequest(const std::string& json);
	virtual ~AdminAuthRequest();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const {return "";};

	std::string code() const {return mCode;};
private:
	std::string mCode;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_ADMINAUTHREQUEST_H_ */
