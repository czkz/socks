#pragma once
#include <string>
#include <stdexcept>
#include <memory>
#include <optional>

#ifdef _WIN32

// Windows socket includes
#include <winsock2.h>
#include <ws2tcpip.h>

#elif __linux__

// Unix socket includes
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#endif


#ifdef _WIN32

namespace SockDefines {
    using socket_t = SOCKET;
    constexpr socket_t null_socket = INVALID_SOCKET;
    inline int get_errno() { return errno; }
    using hostent_t = HOSTENT;
    using sockaddr_len_t = int;

    enum error_codes {
        econnrefused = WSAECONNREFUSED,
        econnreset   = WSAECONNRESET,
        ehostunreach = WSAEHOSTUNREACH,
        enetreset    = WSAENETRESET,
        enetunreach  = WSAENETUNREACH,
        etimedout    = WSAETIMEDOUT,
        ewouldblock  = WSAEWOULDBLOCK,
        no_data      = WSANO_DATA,
    };
}

#elif __linux__

namespace SockDefines {
    using socket_t = int;
    constexpr socket_t null_socket = -1;
    inline int get_errno() { return errno; }
    using hostent_t = hostent;
    using sockaddr_len_t = unsigned int;

    enum error_codes {
        econnrefused = ECONNREFUSED,
        econnreset   = ECONNRESET,
        ehostunreach = EHOSTUNREACH,
        enetreset    = ENETRESET,
        enetunreach  = ENETUNREACH,
        etimedout    = ETIMEDOUT,
        ewouldblock  = EWOULDBLOCK,
        no_data      = NO_DATA,
    };
}

#endif


#ifdef _WIN32
class WSAHandle {
    WSAHandle() {
        WSADATA wsadata;
        if (WSAStartup(0x202, &wsadata)) {
            throw SockError("Socks WSAStartup() failed", &WSAStartup, WSAGetLastError());
        }
    }
    ~WSAHandle() {
        WSACleanup();
    }
};
#elif __linux__
class WSAHandle { };
#endif

class SockHandle {
public:
    SockDefines::socket_t value = SockDefines::null_socket;
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

    explicit SockHandle(const SockDefines::socket_t&) = delete;
    explicit SockHandle(SockDefines::socket_t&& socket);

    ///Returns whether there's data to read / clients to accept
    bool Readable();
};


class SockError : public std::runtime_error {
public:
    const int error_code;
    const void* const func;
    template <typename Func>
    explicit SockError(const char* what, Func func, int error_code)
        : runtime_error(std::string(what) + " [" + std::to_string(error_code) + "]"),
          error_code(SockDefines::get_errno()),
          func((void*)func)
    { }
};

class Host {
    static std::optional<sockaddr_in> GetSockaddr(const char* host, uint16_t port);
public:
    const std::optional<const sockaddr_in> hostInfo;
    const std::string ip;
    const uint16_t port;

    std::string GetHostname() const;

    Host(const sockaddr_in& hostInfo) noexcept;
    Host(std::optional<sockaddr_in>&& hostInfo) noexcept;
    Host(const char* host, uint16_t port);
};
