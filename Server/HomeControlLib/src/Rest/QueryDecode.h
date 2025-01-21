/*
** EPITECH PROJECT, 2025
** HomeControl
** File description:
** QueryDecode
*/

#ifndef QUERYDECODE_H_
#define QUERYDECODE_H_

#include <map>
#include <string>

namespace RestServerNs
{
std::map<std::string, std::string> queryDecode(const std::string& query);
}
#endif /* !QUERYDECODE_H_ */
