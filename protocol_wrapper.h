//
// Created by theevilroot on 2/23/22.
//

#ifndef J_PROTOCOL_WRAPPER_H
#define J_PROTOCOL_WRAPPER_H

#include <variant>

#include "utils.h"
#include "ip_wrapper.h"
#include "time_wrapper.h"


struct TextMessage
{
    sockaddr_in source;
    std::string payload;
    std::string time;
};

struct HelloMessage
{
    sockaddr_in source;
    std::string payload;
};

struct DiscoverMessage
{
    sockaddr_in source;
    std::string payload;
};

typedef std::variant<TextMessage, HelloMessage, DiscoverMessage> ProtocolMessage;

struct ProtocolHeader
{
    std::uint16_t length;
    std::uint16_t flag;
};

struct Protocol
{

    static auto discover(Socket& inet, const std::string& address)
    {
        const std::uint16_t messageSize = address.size();
        const std::size_t packetSize = sizeof(ProtocolHeader) + messageSize;
        ProtocolHeader header{htons(messageSize), htons(3u)};
        char buffer[packetSize];
        std::memcpy(buffer, static_cast<const void*>(&header), sizeof(header));
        std::memcpy(buffer + sizeof(header), address.c_str(), messageSize);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), static_cast<void*>(&addr.sin_addr));
        return inet.sendto(buffer, packetSize, 0, addr);
    }

    static auto hello(Socket& inet, const sockaddr_in& addr, const std::string& payload)
    {
        const std::uint16_t messageSize = payload.size();
        const std::size_t packetSize = messageSize + sizeof(ProtocolHeader);
        ProtocolHeader header{htons(messageSize), htons(2u)};
        char buffer[packetSize];
        std::memcpy(buffer, static_cast<const void*>(&header), sizeof(header));
        std::memcpy(buffer + sizeof(header), payload.c_str(), messageSize);
        return inet.sendto(buffer, packetSize, 0, addr);
    }

    static auto send(Socket& inet, const std::string& message, const std::string& address)
    {
        const std::uint16_t messageSize = message.size();
        const std::size_t packetSize = messageSize + sizeof(ProtocolHeader);
        ProtocolHeader header{htons(messageSize), htons(1u)};
        char buffer[packetSize];
        std::memcpy(buffer, static_cast<const void*>(&header), sizeof(header));
        std::memcpy(buffer + sizeof(header), message.c_str(), messageSize);

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
        ProtocolHeader header{};
        std::memcpy(static_cast<void*>(&header), buffer + 20, sizeof(header));
        header.length = ntohs(header.length);
        header.flag = ntohs(header.flag);
        if (header.flag == 1)
        {
            auto payload = std::string{buffer + 20 + sizeof(header), header.length};
            return TextMessage{addr, payload, time.str};
        }
        else if (header.flag == 2)
        {
            auto payload = std::string{buffer + 20 + sizeof(header), header.length};
            return HelloMessage{addr, payload};
        }
        else
        {
            auto payload = std::string{buffer + 20 + sizeof(header), header.length};
            return DiscoverMessage{addr, payload};
        }
    }
};

#endif //J_PROTOCOL_WRAPPER_H
