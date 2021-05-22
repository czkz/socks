#pragma once
#include "SockPlatform.h"

class SockHandle {
public:
    SockPlatform::socket_t value = SockPlatform::null_socket;
private:
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
    bool Readable() const;

    friend bool operator<(const SockHandle& a, const SockHandle& b);
};

