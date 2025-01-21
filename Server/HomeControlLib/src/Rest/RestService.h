/*
** EPITECH PROJECT, 2025
** homecontrol (Workspace)
** File description:
** RestService
*/

#ifndef RESTSERVICE_H_
#define RESTSERVICE_H_
#include "RestServiceIf.h"
#include <map>

namespace DalNs
{
class HomeControlDalIf;
}

namespace RestServerNs
{
class RestService: public RestServiceIf
{
public:
    RestService(DalNs::HomeControlDalIf* homeControlDal);
	RestService(const RestService&) = delete;
	RestService& operator=(const RestService&) =delete;
	virtual ~RestService();

    void logPosition(const std::string& device, const std::string& timestampStr, const std::string& lat, const std::string& lon, const std::string& batt, const std::string& accuracy);

private:
    DalNs::HomeControlDalIf* mHomeControlDal;
    std::map<std::string, int> mDevices;
};
}




#endif /* !RESTSERVICE_H_ */
