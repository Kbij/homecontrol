/*
** EPITECH PROJECT, 2025
** homecontrol (Workspace)
** File description:
** CppRestServer
*/

#include "CppRestServer.h"
#include "QueryDecode.h"
#include "RestServiceIf.h"
#include <sstream>
#include <glog/stl_logging.h>
#include <glog/logging.h>
#include <boost/algorithm/string.hpp>

namespace RestServerNs
{
CppRestServer::CppRestServer(int listeningPort, RestServiceIf* service):
	mService(service),
	mListener()
{
	std::stringstream address;
	address << "http://0.0.0.0:" <<  listeningPort;
	LOG(INFO) << "Starting rest server on : " << address.str();
	mListener = new http_listener(address.str());
	startListener();

}

CppRestServer::~CppRestServer()
{
	LOG(INFO) << "Stopping Rest server";
	stopListener();
	delete mListener;
}

void CppRestServer::startListener()
{
	mListener->support(methods::GET, std::bind(&CppRestServer::handleGet, this, std::placeholders::_1));
	// mListener->support(methods::POST, std::bind(&RestServer::handlePost, this, std::placeholders::_1));
	// mListener->support(methods::OPTIONS, std::bind(&RestServer::handleOptions, this, std::placeholders::_1));

	mListener->open().wait();
}

void CppRestServer::stopListener()
{
	mListener->close().wait();
}


void CppRestServer::handleGet(http_request message)
{
    VLOG(3) <<  message.to_string();

    std::vector<utility::string_t> paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));
    auto params = queryDecode(message.request_uri().query());
    if (paths.size() == 1 && paths[0] == "log")
    {
        if (mService) mService->logPosition(params["AID"], params["time"], params["lat"], params["lon"], params["BATT"], params["ACC"]);
    }

    message.reply(status_codes::OK);
}

void CppRestServer::handleOptions(http_request message)
{
	http_response response (status_codes::OK);
	response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
	response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
	response.headers().set_content_type("application/json");
	response.set_body("");
	message.reply(response);
}
}
