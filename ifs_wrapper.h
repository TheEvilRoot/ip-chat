//
// Created by theevilroot on 2/22/22.
//

#ifndef J_IFS_WRAPPER_H
#define J_IFS_WRAPPER_H

#include <vector>
#include <string>
#include <utility>

#include <ifaddrs.h>

struct Interface
{
    std::string name;
    std::string ipv4;
    std::string mask;
    std::string broadcast;

    explicit Interface(std::string name, std::string ipv4, std::string mask, std::string broadcast): name{std::move(name)}, ipv4{std::move(ipv4)}, mask{std::move(mask)}, broadcast{std::move(broadcast)}
    {
    }

    static auto interfaces()
    {
        ifaddrs *list{};
        getifaddrs(&list);
        std::vector<Interface> ifs{};
        for (auto* i = list; i != nullptr; i = i->ifa_next)
        {
            if (i->ifa_addr->sa_family != AF_INET || i->ifa_addr->sa_family == AF_INET6)
                continue;
            auto name = std::string{i->ifa_name};
            char buffer[40];
            auto in4 = reinterpret_cast<const sockaddr_in*>(i->ifa_addr)->sin_addr;
            auto in6 = reinterpret_cast<const sockaddr_in6*>(i->ifa_addr)->sin6_addr;
            if (i->ifa_addr->sa_family == AF_INET)
                inet_ntop(i->ifa_addr->sa_family, static_cast<const void*>(&in4), buffer, sizeof(buffer));
            else inet_ntop(i->ifa_addr->sa_family, static_cast<const void*>(&in6), buffer, sizeof(buffer));

            auto ip = std::string{buffer};
            auto mask4 = reinterpret_cast<const sockaddr_in*>(i->ifa_netmask)->sin_addr;
            auto mask6 = reinterpret_cast<const sockaddr_in6*>(i->ifa_netmask)->sin6_addr;
            if (i->ifa_netmask->sa_family == AF_INET)
                inet_ntop(i->ifa_netmask->sa_family, static_cast<const void*>(&mask4), buffer, sizeof(buffer));
            else inet_ntop(i->ifa_netmask->sa_family, static_cast<const void*>(&mask6), buffer, sizeof(buffer));

            auto mask = std::string{buffer};
            auto dst4 = reinterpret_cast<const sockaddr_in*>(i->ifa_dstaddr)->sin_addr;
            auto dst6 = reinterpret_cast<const sockaddr_in6*>(i->ifa_dstaddr)->sin6_addr;
            if (i->ifa_dstaddr->sa_family == AF_INET)
                inet_ntop(i->ifa_dstaddr->sa_family, static_cast<const void*>(&dst4), buffer, sizeof(buffer));
            else inet_ntop(i->ifa_dstaddr->sa_family, static_cast<const void*>(&dst6), buffer, sizeof(buffer));

            auto dst = std::string{buffer};
            ifs.emplace_back(name, utils::addr_to_string(*i->ifa_addr), utils::addr_to_string(*i->ifa_netmask), utils::addr_to_string(*i->ifa_dstaddr));
        }
        return ifs;
    }
};

#endif //J_IFS_WRAPPER_H
