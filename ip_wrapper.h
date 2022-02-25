//
// Created by theevilroot on 2/23/22.
//

#ifndef J_IP_WRAPPER_H
#define J_IP_WRAPPER_H

template<std::size_t Size>
struct IpPacket
{
    std::uint8_t version;
    std::uint8_t dsf;
    std::uint16_t length;
    std::uint16_t identification;
    std::uint16_t flags;
    std::uint8_t ttl;
    std::uint8_t proto;
    std::uint16_t checksum;
    std::uint32_t source;
    std::uint32_t dest;
    char data[Size];
};

#endif //J_IP_WRAPPER_H
