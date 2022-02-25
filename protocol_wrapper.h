//
// Created by theevilroot on 2/23/22.
//

#ifndef J_PROTOCOL_WRAPPER_H
#define J_PROTOCOL_WRAPPER_H

#include "utils.h"
#include "ip_wrapper.h"
#include "time_wrapper.h"

struct ProtocolMessage
{
    sockaddr_in source;
    std::string payload;
    std::string time;
};

struct Protocol
{
    static auto send(Socket& inet, const std::string& message, const std::string& address)
    {
        const std::uint16_t messageSize = message.size();
        const std::size_t packetSize = messageSize + 2;
        char buffer[packetSize];
        const std::uint16_t x = htons(messageSize);
        std::memcpy(buffer, static_cast<const void*>(&x), 2);
        std::memcpy(buffer + 2, message.c_str(), messageSize);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), static_cast<void*>(&addr.sin_addr));
        return inet.sendto(buffer, packetSize, 0, addr);
    }

    static ProtocolMessage recv(Socket& inet, char* buffer, const std::size_t bufferSize)
    {
        sockaddr_in addr{};
        const auto size = inet.recvfrom(buffer, bufferSize, 0, addr);
        const auto time = Time{};
        std::uint16_t x{};
        std::memcpy(static_cast<void*>(&x), buffer + 20, 2);
        x = ntohs(x);
        auto payload = std::string{buffer + 20 + 2, x};
        return ProtocolMessage{addr, payload, time.str};
    }
};

#endif //J_PROTOCOL_WRAPPER_H
