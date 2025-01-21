/*
** EPITECH PROJECT, 2025
** homecontrol (Workspace)
** File description:
** CppRestServer
*/

#ifndef CPPRESTSERVER_H_
#define CPPRESTSERVER_H_
#include "cpprest/http_listener.h"

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

namespace RestServerNs
{
class RestServiceIf;

class CppRestServer
{
public:
	CppRestServer(int listeningPort, RestServiceIf* service);
	CppRestServer(const CppRestServer&) = delete;
	CppRestServer& operator=(const CppRestServer&) =delete;
	virtual ~CppRestServer();
private:
	void startListener();
	void stopListener();
	void handleGet(http_request message);
	// void handlePost(http_request message);
	void handleOptions(http_request message);

	RestServiceIf* mService;
	http_listener* mListener;
};
}

#endif /* !CPPRESTSERVER_H_ */
