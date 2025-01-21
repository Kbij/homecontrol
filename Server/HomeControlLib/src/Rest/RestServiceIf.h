/*
** EPITECH PROJECT, 2025
** homecontrol (Workspace)
** File description:
** RestServiceIf
*/

#ifndef RESTSERVICEIF_H_
#define RESTSERVICEIF_H_
#include <string>

namespace RestServerNs
{
class RestServiceIf
{
public:
    virtual ~RestServiceIf(){};

    virtual void logPosition(const std::string& device, const std::string& timestampStr,  const std::string& lat, const std::string& lon, const std::string& batt, const std::string& accuracy) = 0;
};
}

#endif /* !RESTSERVICEIF_H_ */
