/*
 * AdminAuthResult.h
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#ifndef COMMOBJECTS_ADMINAUTHRESULT_H_
#define COMMOBJECTS_ADMINAUTHRESULT_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>

namespace CommNs {

/**
 * Server -> client, objectId 41. Reply to AdminAuthRequest (and also (re)sent, with
 * success=false, whenever a later privileged request turns out not to be authorized -
 * see AdminController::handleLocationHistoryRequest). Payload: JSON {"success": <bool>}.
 */
class AdminAuthResult: public CommObjectIf
{
public:
	AdminAuthResult(bool success);
	virtual ~AdminAuthResult();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;
private:
	bool mSuccess;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_ADMINAUTHRESULT_H_ */
