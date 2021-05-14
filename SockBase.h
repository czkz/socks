#pragma once
#include <string>
#include <stdexcept>
#include <memory>
#include <optional>

#include "SockPlatform.h"

class SockHandle {
public:
    SockPlatform::socket_t value = SockPlatform::null_socket;
private:
    fd_set thisSet;
    WSAHandle wsa_handle;
public:
    SockHandle(int af, int type, int protocol);
    ~SockHandle();

    // No copy, only move
    SockHandle(const SockHandle&) = delete;
    SockHandle& operator=(const SockHandle&) = delete;
    SockHandle(SockHandle&& other);
    SockHandle& operator=(SockHandle&& other);

    explicit SockHandle(const SockPlatform::socket_t&) = delete;
    explicit SockHandle(SockPlatform::socket_t&& socket);

    ///Returns whether there's data to read / clients to accept
    bool Readable();
};


class SockError : public std::runtime_error {
public:
    const int error_code;
    const void* const func;

    template <typename Func>
    SockError(const char* what, Func func, int error_code)
        : runtime_error(std::string(what) + " [" + std::to_string(error_code) + "]"),
          error_code(SockPlatform::get_errno()),
          func((void*)func) { }
};

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
