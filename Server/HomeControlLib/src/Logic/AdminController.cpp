/*
 * AdminController.cpp
 *
 *  Created on: Sep 6, 2026
 *      Author: koen
 */

#include "Logic/AdminController.h"
#include "Comm/CommServerIf.h"
#include "DAL/HomeControlDalIf.h"
#include "CommObjects/AdminAuthRequest.h"
#include "CommObjects/AdminAuthResult.h"
#include "CommObjects/ClientList.h"
#include "CommObjects/LocationHistoryRequest.h"
#include "CommObjects/LocationHistoryResponse.h"
#include "CommObjects/GeofenceStatus.h"
#include "CommObjects/GpsLocation.h"
#include <glog/logging.h>

namespace LogicNs {

AdminController::AdminController(DalNs::HomeControlDalIf* dal, CommNs::CommServerIf* server):
	mDal(dal),
	mCommServer(server)
{
	if (mCommServer) mCommServer->registerCommListener(this);
}

AdminController::~AdminController()
{
	if (mCommServer) mCommServer->unRegisterCommListener(this);
}

void AdminController::clientConnected(const std::string& name)
{
	// No per-connection admin state is kept (see isAdmin()), so there's nothing to do here.
}

void AdminController::clientDisConnected(const std::string& name)
{
	// Stop pushing to a watcher that's gone - also covers an admin connection that dropped
	// without a clean LocationHistoryRequest-based handoff (e.g. app backgrounded/killed);
	// Server::maintenanceThread calls this once the connection is reaped as inactive.
	std::lock_guard<std::mutex> lg(mWatchersMutex);
	mWatchers.erase(name);
}

void AdminController::receiveObject(const std::string name, const CommNs::CommObjectIf* object)
{
	if (object->objectId() == 40)
	{
		handleAuthRequest(name, object);
	}
	if (object->objectId() == 43)
	{
		handleLocationHistoryRequest(name, object);
	}
	if (object->objectId() == 45)
	{
		handleGeofenceStatus(name, object);
		// DB write already happened synchronously just above, no race - see
		// notifyLocationWatchers()'s doc comment.
		notifyLocationWatchers(name, nullptr);
	}
	if (object->objectId() == 10)
	{
		if (const CommNs::GpsLocation* location = dynamic_cast<const CommNs::GpsLocation*>(object))
		{
			notifyLocationWatchers(name, location);
		}
	}
	if (object->objectId() == 0)
	{
		// KeepAlive: no location/geofence data changed, but Client.lastMessage just did (see
		// ObjectWriter::receiveObject, which updates it on every message) - push so "laatste
		// connectie" on the admin map stays live even while the client is stationary and not
		// otherwise triggering a push (no fresh GpsLocation to hand notifyLocationWatchers here,
		// same as the GeofenceStatus case - a few ms of staleness on this field is harmless).
		notifyLocationWatchers(name, nullptr);
	}
}

bool AdminController::isAdmin(const std::string& clientName)
{
	if (!mDal) return false;

	// Re-checked live against the database on every call, rather than remembering
	// "this connection already sent a correct AdminAuthRequest once" - adminCode can be
	// cleared server-side at any time, and a stale in-memory grant would keep working
	// until the connection dropped. Capability-gated, not session-gated.
	return !mDal->adminCode(clientName).empty();
}

void AdminController::handleAuthRequest(const std::string& requester, const CommNs::CommObjectIf* object)
{
	if (const CommNs::AdminAuthRequest* request = dynamic_cast<const CommNs::AdminAuthRequest*>(object))
	{
		if (!mDal || !mCommServer) return;

		std::string expectedCode = mDal->adminCode(requester);
		bool success = !expectedCode.empty() && expectedCode == request->code();

		LOG(INFO) << "Admin auth attempt from: " << requester << ", success: " << success;

		//CommServer takes ownership of the object (and free's the object)
		mCommServer->sendObject(requester, new CommNs::AdminAuthResult(success));

		if (success)
		{
			CommNs::ClientList* clientList = new CommNs::ClientList();
			for (const auto& clientName: mDal->allClientNames())
			{
				clientList->addClient(clientName);
			}

			//CommServer takes ownership of the object (and free's the object)
			mCommServer->sendObject(requester, clientList);
		}
	}
}

void AdminController::handleLocationHistoryRequest(const std::string& requester, const CommNs::CommObjectIf* object)
{
	if (const CommNs::LocationHistoryRequest* request = dynamic_cast<const CommNs::LocationHistoryRequest*>(object))
	{
		if (!mDal || !mCommServer) return;

		if (!isAdmin(requester))
		{
			LOG(WARNING) << "LocationHistoryRequest from non-admin client: " << requester << ", denying";
			{
				std::lock_guard<std::mutex> lg(mWatchersMutex);
				mWatchers.erase(requester);
			}
			//CommServer takes ownership of the object (and free's the object)
			mCommServer->sendObject(requester, new CommNs::AdminAuthResult(false));
			return;
		}

		int minutes = request->minutes() > 0 ? request->minutes() : 60;
		LOG(INFO) << "Location history requested by: " << requester << ", for client: " << request->clientName() << ", minutes: " << minutes;

		{
			std::lock_guard<std::mutex> lg(mWatchersMutex);
			mWatchers[requester] = Watch{request->clientName(), minutes};
		}

		sendLocationHistoryResponse(requester, request->clientName(), minutes, nullptr);
	}
}

void AdminController::handleGeofenceStatus(const std::string& requester, const CommNs::CommObjectIf* object)
{
	if (const CommNs::GeofenceStatus* status = dynamic_cast<const CommNs::GeofenceStatus*>(object))
	{
		if (!mDal) return;

		if (status->active())
		{
			LOG(INFO) << "Geofence reported for client: " << requester << ", lat: " << status->lat()
				<< ", lon: " << status->lon() << ", radius: " << status->radius();
			mDal->updateGeofence(requester, status->lat(), status->lon(), status->radius());
		}
		else
		{
			LOG(INFO) << "Geofence cleared for client: " << requester;
			mDal->clearGeofence(requester);
		}
	}
}

void AdminController::sendLocationHistoryResponse(const std::string& requester, const std::string& clientName,
	int minutes, const CommNs::GpsLocation* freshLocation)
{
	if (!mDal || !mCommServer) return;

	CommNs::LocationHistoryResponse* response = new CommNs::LocationHistoryResponse(clientName);
	for (const auto& point: mDal->locationHistory(clientName, minutes))
	{
		CommNs::LocationPoint commPoint;
		commPoint.Latitude = point.Latitude;
		commPoint.Longitude = point.Longitude;
		commPoint.Timestamp = point.Timestamp;
		response->addPoint(commPoint);
	}

	if (freshLocation)
	{
		// The fix that triggered this push may not be committed to the Location table yet -
		// ObjectWriter (which does that INSERT) and AdminController are both plain
		// CommListenerIf's on the same Server::receiveObject broadcast, and Server iterates
		// them via a std::set<CommListenerIf*> - ordered by pointer value, not registration
		// order, so there's no guarantee ObjectWriter runs first. Appending it explicitly here,
		// straight from the object already in hand, sidesteps that race entirely instead of
		// hoping the DB write landed first.
		CommNs::LocationPoint freshPoint;
		freshPoint.Latitude = freshLocation->latitude();
		freshPoint.Longitude = freshLocation->longitude();
		freshPoint.Timestamp = freshLocation->timeStamp();
		response->addPoint(freshPoint);
	}

	DalNs::GeofenceInfo geofence = mDal->geofence(clientName);
	if (geofence.Active)
	{
		response->setGeofence(true, geofence.Latitude, geofence.Longitude, geofence.RadiusMeters,
			geofence.CreatedAt, geofence.UpdatedAt);
	}

	response->setLastConnection(mDal->lastMessage(clientName));

	//CommServer takes ownership of the object (and free's the object)
	mCommServer->sendObject(requester, response);
}

void AdminController::notifyLocationWatchers(const std::string& clientName, const CommNs::GpsLocation* freshLocation)
{
	std::vector<std::pair<std::string, int>> watchers;
	{
		std::lock_guard<std::mutex> lg(mWatchersMutex);
		for (const auto& entry: mWatchers)
		{
			if (entry.second.clientName == clientName)
			{
				watchers.push_back({entry.first, entry.second.minutes});
			}
		}
	}

	for (const auto& watcher: watchers)
	{
		sendLocationHistoryResponse(watcher.first, clientName, watcher.second, freshLocation);
	}
}

} /* namespace LogicNs */
