/*
 * AdminController.h
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#ifndef LOGIC_ADMINCONTROLLER_H_
#define LOGIC_ADMINCONTROLLER_H_
#include "Comm/CommListenerIf.h"
#include <string>

namespace CommNs
{
class CommServerIf;
class CommObjectIf;
}
namespace DalNs
{
class HomeControlDalIf;
}
namespace LogicNs {

/**
 * Handles the two client -> server admin-mode messages (AdminAuthRequest, objectId 40,
 * and LocationHistoryRequest, objectId 43). Not gated by session state: every request is
 * independently re-checked against DalNs::HomeControlDalIf::adminCode, live, rather than
 * trusting a "this connection already authenticated once" flag - see isAdmin() below.
 * The PIN itself never leaves the server.
 */
class AdminController: public CommNs::CommListenerIf
{
public:
	AdminController(DalNs::HomeControlDalIf* dal, CommNs::CommServerIf* server);
	virtual ~AdminController();

	//CommListenerIf
	void clientConnected(const std::string& name);
	void clientDisConnected(const std::string& name);
	void receiveObject(const std::string name, const CommNs::CommObjectIf* object);

private:
	DalNs::HomeControlDalIf* mDal;
	CommNs::CommServerIf* mCommServer;

	void handleAuthRequest(const std::string& requester, const CommNs::CommObjectIf* object);
	void handleLocationHistoryRequest(const std::string& requester, const CommNs::CommObjectIf* object);
	bool isAdmin(const std::string& clientName);
};

} /* namespace LogicNs */

#endif /* LOGIC_ADMINCONTROLLER_H_ */
