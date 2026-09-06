/*
 * ClientList.h
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#ifndef COMMOBJECTS_CLIENTLIST_H_
#define COMMOBJECTS_CLIENTLIST_H_
#include "CommObjectIf.h"
#include <string>
#include <stdint.h>
#include <vector>

namespace CommNs {

/**
 * Server -> client, objectId 42. Sent right after a successful AdminAuthResult, so the
 * admin screen has something to populate its client picker with. Lists every client
 * known to the database (DalNs::HomeControlDalIf::allClientNames), not only the
 * currently-connected ones - a client that's briefly offline still has recent location
 * history worth looking at.
 */
class ClientList: public CommObjectIf
{
public:
	ClientList();
	virtual ~ClientList();

	// CommObjectIf
	uint8_t objectId() const;
	std::string toString() const;
	std::string json() const;

	void addClient(const std::string& name);

private:
	std::vector<std::string> mClients;
};

} /* namespace CommNs */

#endif /* COMMOBJECTS_CLIENTLIST_H_ */
