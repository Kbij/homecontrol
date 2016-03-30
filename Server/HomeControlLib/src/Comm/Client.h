/*
 * Client.h
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#ifndef COMM_CLIENT_H_
#define COMM_CLIENT_H_
#include <boost/asio.hpp>

namespace CommNs {
class ClientSocketIf;

class Client {
public:
	Client(ClientSocketIf* clientSocket);
	virtual ~Client();

	boost::asio::ip::tcp::tcp::socket& socket();
	void start();
private:
	ClientSocketIf* mClientSocket;
};

} /* namespace CommNs */

#endif /* COMM_CLIENT_H_ */
