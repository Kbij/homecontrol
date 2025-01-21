/*
** EPITECH PROJECT, 2025
** HomeControl
** File description:
** QueryDecode
*/

#include "QueryDecode.h"
#include <vector>
#include <boost/algorithm/string.hpp>

namespace RestServerNs
{
std::map<std::string, std::string> queryDecode(const std::string& query)
{
    std::map<std::string, std::string> result;

    std::vector<std::string> params;
    boost::split(params, query, boost::is_any_of("&"));
    for(const std::string& param: params)
    {
        auto pos = param.find('=');
        if (pos != std::string::npos)
        {
            result[param.substr(0, pos)] = param.substr(pos + 1, param.size() - pos - 1);
        }
    }
    return result;
}
}
