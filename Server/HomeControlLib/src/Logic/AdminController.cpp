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
	// Nothing to clean up - see clientConnected().
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
			//CommServer takes ownership of the object (and free's the object)
			mCommServer->sendObject(requester, new CommNs::AdminAuthResult(false));
			return;
		}

		int minutes = request->minutes() > 0 ? request->minutes() : 60;
		LOG(INFO) << "Location history requested by: " << requester << ", for client: " << request->clientName() << ", minutes: " << minutes;

		CommNs::LocationHistoryResponse* response = new CommNs::LocationHistoryResponse(request->clientName());
		for (const auto& point: mDal->locationHistory(request->clientName(), minutes))
		{
			CommNs::LocationPoint commPoint;
			commPoint.Latitude = point.Latitude;
			commPoint.Longitude = point.Longitude;
			commPoint.Timestamp = point.Timestamp;
			response->addPoint(commPoint);
		}

		DalNs::GeofenceInfo geofence = mDal->geofence(request->clientName());
		if (geofence.Active)
		{
			response->setGeofence(true, geofence.Latitude, geofence.Longitude, geofence.RadiusMeters,
				geofence.CreatedAt, geofence.UpdatedAt);
		}

		//CommServer takes ownership of the object (and free's the object)
		mCommServer->sendObject(requester, response);
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

} /* namespace LogicNs */
