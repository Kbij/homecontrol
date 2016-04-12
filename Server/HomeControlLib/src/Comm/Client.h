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
enum class ConnectionState {Idle, Connecting, Connected, Disconnected};

class Client: public SocketListenerIf
{
public:
	// Client becomes owner of clientSocket
	Client(ClientSocketIf* clientSocket, ClientListenerIf* clientListener);
	virtual ~Client();

	boost::asio::ip::tcp::tcp::socket& socket();
	void start();
	bool isInactive(int milliSecondsPassed);

	//SocketListenerIf
	std::string name() const {return mName;};
	void receiveFrame(uint8_t objectId, const std::vector<uint8_t>& frame);
	void socketClosed();

private:
	ClientSocketIf* mClientSocket;
	ClientListenerIf* mClientListener;
	std::string mName;
	ConnectionState mConnectionState;
	int mConnectingTime;
	int mLastFrameTime;
};

} /* namespace CommNs */

#endif /* COMM_CLIENT_H_ */
