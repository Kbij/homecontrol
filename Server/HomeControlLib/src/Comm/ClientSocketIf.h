/*
 * ClientIf.h
 *
 *  Created on: Mar 29, 2016
 *      Author: koen
 */

#ifndef COMM_CLIENTSOCKETIF_H_
#define COMM_CLIENTSOCKETIF_H_
#include <boost/asio.hpp>
#include <string>

namespace CommNs {
class SocketListenerIf;

class ClientSocketIf
{
public:
	virtual ~ClientSocketIf() {};

	virtual boost::asio::ip::tcp::tcp::socket& socket() = 0;
	virtual void start() = 0;
	virtual std::string name() = 0;

	virtual void registerSocketListener(SocketListenerIf* socketListener) = 0;
	virtual void sendFrame(uint8_t objectId, const std::vector<uint8_t>& frame) = 0;
};

} /* namespace CommNs */


#endif /* COMM_CLIENTSOCKETIF_H_ */
