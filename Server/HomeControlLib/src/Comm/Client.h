/*
 * Client.h
 *
 *  Created on: Mar 30, 2016
 *      Author: koen
 */

#ifndef COMM_CLIENT_H_
#define COMM_CLIENT_H_
#include "SocketListenerIf.h"
#include <boost/asio.hpp>


namespace CommNs
{
class ClientSocketIf;
class ClientListenerIf;
enum class ConnectionState {Initiated, Connected, Disconnected};

class Client: public SocketListenerIf
{
public:
	Client(ClientSocketIf* clientSocket, ClientListenerIf* clientListener);
	virtual ~Client();

	boost::asio::ip::tcp::tcp::socket& socket();
	void start();
	void receiveFrame(uint8_t objectId, const std::vector<uint8_t>& frame);
	std::string name() const {return mName;};
	bool connected() {return mConnectionState == ConnectionState::Connected;};
private:
	ClientSocketIf* mClientSocket;
	ClientListenerIf* mClientListener;
	std::string mName;
	ConnectionState mConnectionState;
};

} /* namespace CommNs */

#endif /* COMM_CLIENT_H_ */
