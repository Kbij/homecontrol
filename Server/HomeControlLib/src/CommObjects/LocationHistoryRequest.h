/*
 * LocationHistoryRequest.h
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#ifndef COMMOBJECTS_LOCATIONHISTORYREQUEST_H_
#define COMMOBJECTS_LOCATIONHISTORYREQUEST_H_
#include "CommObjectIf.h"
#include <string>

namespace CommNs {

/**
 * Client -> server, objectId 43. Payload: JSON {"client": "<name>", "minutes": <int>}.
 * Sent by the admin screen whenever the selected client changes. Authorization is not
 * implicit from having sent a prior AdminAuthRequest - AdminController re-checks the
 * *requester's* own admin capability live on every one of these (see AdminController).
 */
class LocationHistoryRequest: public CommObjectIf
{
public:
	LocationHistoryRequest(const std::string& json);
	virtual ~LocationHistoryRequest();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const {return "";};

	/** Name of the client whose location history is being requested (not necessarily the requester). */
	std::string clientName() const {return mClientName;};
	/** How many minutes of history to return; <= 0 means "use the default (60)". */
	int minutes() const {return mMinutes;};
private:
	std::string mClientName;
	int mMinutes;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_LOCATIONHISTORYREQUEST_H_ */
