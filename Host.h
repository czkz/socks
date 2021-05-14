#pragma once
#include <string>
#include <optional>
#include "SockPlatform.h"

class Host {
    static std::optional<sockaddr_in> getSockaddr(const char* host, uint16_t port);
public:
    const std::optional<const sockaddr_in> hostInfo;
    const std::string ip;
    const uint16_t port;

    std::string GetHostname() const;

    Host(const sockaddr_in& hostInfo) noexcept;
    Host(std::optional<sockaddr_in>&& hostInfo) noexcept;
    Host(const char* host, uint16_t port);
};
