//
// Created by theevilroot on 2/23/22.
//

#ifndef J_UTILS_H
#define J_UTILS_H

#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct utils
{

    static std::string addr_to_string(const sockaddr_in& addr)
    {
        char buffer[17]{0};
        inet_ntop(AF_INET, static_cast<const void*>(&addr.sin_addr), buffer, sizeof(buffer));
        return std::string{buffer};
    }

    static std::string addr_to_string(const sockaddr_in6& addr)
    {
        char buffer[40]{0};
        inet_ntop(AF_INET6, static_cast<const void*>(&addr.sin6_addr), buffer, sizeof(buffer));
        return std::string{buffer};
    }

    static std::string addr_to_string(const sockaddr& addr)
    {
        if (addr.sa_family == AF_INET)
        {
            return addr_to_string(*reinterpret_cast<const sockaddr_in*>(&addr));
        }
        else
        {
            return addr_to_string(*reinterpret_cast<const sockaddr_in6*>(&addr));
        }
    }
};

#endif //J_UTILS_H
