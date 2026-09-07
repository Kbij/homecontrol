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
#include <map>
#include <mutex>
#include <utility>
#include <vector>

namespace CommNs
{
class CommServerIf;
class CommObjectIf;
class GpsLocation;
}
namespace DalNs
{
class HomeControlDalIf;
}
namespace LogicNs {

/**
 * Handles the client -> server admin-mode/geofencing messages: AdminAuthRequest (objectId
 * 40), LocationHistoryRequest (objectId 43), and GeofenceStatus (objectId 45). The first two
 * are not gated by session state: every request is independently re-checked against
 * DalNs::HomeControlDalIf::adminCode, live, rather than trusting a "this connection already
 * authenticated once" flag - see isAdmin() below. The PIN itself never leaves the server.
 * GeofenceStatus, unlike the other two, isn't admin-gated at all - it's ordinary telemetry
 * from a client about itself (same trust level as GpsLocation), reported over the *primary*
 * TCP connection, not the admin one.
 *
 * Also live-pushes LocationHistoryResponse (objectId 44) to admin connections that are
 * "watching" a client: whenever that client's GpsLocation (objectId 10) or GeofenceStatus
 * (objectId 45) is seen going by on Server::receiveObject's broadcast, every admin connection
 * that most recently sent a LocationHistoryRequest for that client gets a fresh response
 * without having to ask again - see mWatchers / notifyLocationWatchers().
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
	/** One admin connection's most recent LocationHistoryRequest: which client it's watching,
	 *  and the requested window - both re-used for every live push, so a push always reflects
	 *  the same window the admin last asked for. */
	struct Watch
	{
		std::string clientName;
		int minutes;
	};

	DalNs::HomeControlDalIf* mDal;
	CommNs::CommServerIf* mCommServer;
	std::map<std::string, Watch> mWatchers; // keyed by the admin connection's own name
	std::mutex mWatchersMutex;

	void handleAuthRequest(const std::string& requester, const CommNs::CommObjectIf* object);
	void handleLocationHistoryRequest(const std::string& requester, const CommNs::CommObjectIf* object);
	void handleGeofenceStatus(const std::string& requester, const CommNs::CommObjectIf* object);
	bool isAdmin(const std::string& clientName);

	/** Sends a fresh LocationHistoryResponse for clientName to requester. [freshLocation], when
	 *  given, is appended to the DB-sourced points explicitly rather than relied upon already
	 *  being in the DB - see notifyLocationWatchers() for why. */
	void sendLocationHistoryResponse(const std::string& requester, const std::string& clientName,
		int minutes, const CommNs::GpsLocation* freshLocation);

	/** Pushes an updated LocationHistoryResponse to every admin connection currently watching
	 *  clientName (see mWatchers). [freshLocation] is passed through to
	 *  sendLocationHistoryResponse() - pass it (non-null) when this was triggered by a just-
	 *  received GpsLocation, since ObjectWriter (which commits it to the DB) and this class are
	 *  both plain CommListenerIf's on the same broadcast with no ordering guarantee between
	 *  them, so the DB may not have the new point yet; pass nullptr when triggered by a
	 *  GeofenceStatus, since that DB write already happened synchronously in this same call
	 *  (handleGeofenceStatus, called just before). */
	void notifyLocationWatchers(const std::string& clientName, const CommNs::GpsLocation* freshLocation);
};

} /* namespace LogicNs */

#endif /* LOGIC_ADMINCONTROLLER_H_ */
