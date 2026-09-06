/*
 * AdminCapability.h
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#ifndef COMMOBJECTS_ADMINCAPABILITY_H_
#define COMMOBJECTS_ADMINCAPABILITY_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>

namespace CommNs
{

/**
 * Server -> client, objectId 39. Pushed from Server::maintenanceThread whenever this
 * client's Client.adminCode row goes from empty to non-empty (or back), mirroring how
 * LocationInterval is pushed. Tells the app whether it's allowed to show the admin
 * (swipe-left) screen at all - it is *not* itself a security boundary: every privileged
 * request (AdminAuthRequest / LocationHistoryRequest) is independently re-checked
 * against the database, so this is purely a UI affordance.
 */
class AdminCapability: public CommObjectIf
{
public:
	AdminCapability(bool isAdmin);
	virtual ~AdminCapability();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;
private:
	bool mIsAdmin;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_ADMINCAPABILITY_H_ */
