//
// Created by theevilroot on 2/22/22.
//

#ifndef J_SOCKET_H
#define J_SOCKET_H

#include <cstring>
#include <cerrno>
#include <string>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>

#include "error_wrapper.h"

struct Socket
{
    int handle{-1};
    Error error{};

    std::size_t sent{};
    std::size_t recved{};

    explicit Socket(int type, int proto)
    {
        handle = socket(AF_INET, type, proto);
        if (handle < 0)
        {
            error = Error("socket");
        }
        else
        {
            int broadcast = 1;
            if (setsockopt(handle, SOL_SOCKET, SO_BROADCAST, static_cast<const void*>(&broadcast), sizeof(broadcast)) != 0)
            {
                error = Error("setsockopt(SO_BROADCAST)");
            }
        }
    }

    auto sendto(const char* buffer, const std::size_t buffer_size, int flags, const sockaddr_in& addr)
    {
        auto res = ::sendto(handle, static_cast<const void*>(buffer), buffer_size, flags, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
        if (res < 0)
        {
            error = Error("sendto");
        }
        sent += res;
        return res;
    }

    auto recvfrom(char* buffer, const std::size_t buffer_size, int flags, sockaddr_in& addr)
    {
        socklen_t len = sizeof(addr);
        auto res = ::recvfrom(handle, static_cast<void*>(buffer), buffer_size, flags, reinterpret_cast<sockaddr*>(&addr), &len);
        if (res < 0)
        {
            error = Error("recvfrom");
        }
        recved += res;
        return res;
    }

    void close() const
    {
        ::close(handle);
    }
};

#endif //J_SOCKET_H
