#pragma once
#include <string>
#include "SockHandle.h"
#include "Host.h"
#include "SockError.h"


class SockStreamBase {
protected:
    SockHandle sock;

protected:
    SockStreamBase() : sock(AF_INET, SOCK_STREAM, IPPROTO_TCP) { }
    SockStreamBase(SockPlatform::socket_t&& sock) : sock(std::move(sock)) { }

public:
    /// Sends a FIN packet
    void Disconnect() const;

    friend bool operator<(const SockStreamBase& a, const SockStreamBase& b) { return a.sock < b.sock; }
};

class SockConnection : public SockStreamBase {
protected:
    using SockStreamBase::SockStreamBase;

    /// @return Bytes received, 0 if would block (if nonblocking)
    /// @param shouldFill Whether to wait for the buffer to fill up completely
    int receiveBase(void* buffer, int bufferLength, bool shouldFill) const;

    /// @param n Amount of bytes to receive
    std::string receiveString(size_t) const;
    std::string receiveAvailable() const;
    std::string receiveUntilFIN() const;

public:
    void Send(const void* data, int dataLength) const;

    template <typename Container>
    inline void Send(const Container& c) const { Send(std::data(c), std::size(c)); }


    void ReceiveInto(void* buffer, size_t bufferLength);
    inline std::string ReceiveAvailable() const { return receiveAvailable(); }
    inline std::string ReceiveFill(size_t n) const { return receiveString(n); }
    /// Receive until a FIN packet
    inline std::string ReceiveAll() const { return receiveUntilFIN(); }

    inline bool HasData() const { return sock.Readable(); }
};


class SockClient : public SockConnection {
public:
    bool Connect(const char* host, uint16_t port) const;
};


class ClientConnection : public SockConnection {
public:
    Host host;
protected:
    ClientConnection(SockPlatform::socket_t&& in_sock, const sockaddr_in& clientHostInfo)
        : SockConnection(std::move(in_sock)), host(clientHostInfo) { }
    friend class SockServer;
};

class SockServer : public SockStreamBase {
public:
    /// @param backlog Max length of the pending connections queue
    void Start(uint16_t port, int backlog = 256) const;
    ClientConnection Accept() const;
    inline bool HasClients() const { return sock.Readable(); }
};


class SockDisconnect : public SockError {
public:
    using SockError::SockError;
    template <typename Func>
    explicit SockDisconnect(Func func, int wsaerror) : SockError("Lost connection to remote host", func, wsaerror) { }
};
