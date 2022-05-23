/*
 * ObjectWriter.h
 *
 *  Created on: Apr 8, 2016
 *      Author: koen
 */

#ifndef DAL_OBJECTWRITER_H_
#define DAL_OBJECTWRITER_H_
#include "Comm/CommListenerIf.h"


namespace CommNs
{
class CommObjectIf;
}
namespace DalNs {

class ObjectWriter: public CommNs::CommListenerIf
{
public:
	ObjectWriter(const std::string& server, int port, const std::string& db, const std::string& user, const std::string& pwd);
	virtual ~ObjectWriter();

	//CommListenerIf
	void clientConnected(const std::string& name);
	void clientDisConnected(const std::string& name);
	void receiveObject(const std::string name, const CommNs::CommObjectIf* object);
private:
	const std::string mServer;
	const int mPort;
	const std::string mDb;
	const std::string mUser;
	const std::string mPwd;
};

} /* namespace DalNs */

#endif /* DAL_OBJECTWRITER_H_ */
