#pragma once
#include <string>
#include <stdexcept>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <memory>
#include <optional>

class SockBase {
protected:
    class SockResource {
    private:
        fd_set thisSet;
    public:
        SOCKET sock = INVALID_SOCKET;

    public:
        SockResource(int af, int type, int protocol);
        ~SockResource();

        //Disable copy and assignment, only allow move ctor
        SockResource(const SockResource&) = delete;
        SockResource& operator=(const SockResource&) = delete;
        SockResource& operator=(SockResource&&) = delete;
        SockResource(SockResource&& other) : SockResource(std::move(other.sock)) { }

        explicit SockResource(const SOCKET&) = delete;
        explicit SockResource(SOCKET&& socket);

        ///Returns whether there's data to read / clients to accept
        bool Readable();
    };
    std::shared_ptr<SockResource> sockrc;
    SockBase(int af, int type, int protocol) : sockrc(std::make_shared<SockResource>(af, type, protocol)) { }
    SockBase(SOCKET&& sock) : sockrc(std::make_shared<SockResource>(std::move(sock))) { }
    SockBase(SockResource&& rc) : SockBase(std::move(rc.sock)) { }
    SockBase(SockBase&& other) = default;
};


class SockError : public std::runtime_error {
public:
    const int wsaerror;
    const void* const func;
    template <typename function>
    explicit SockError(const char* what, function func, int wsaerror)
        : runtime_error(std::string(what) + " [" + std::to_string(wsaerror) + "]"),
          wsaerror(WSAGetLastError()),
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
