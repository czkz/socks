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
    ///Initialize graceful disconnect sequence.
    void Disconnect() const;
};

class SockConnection : public SockStreamBase {
protected:
    using SockStreamBase::SockStreamBase;

    ///@return Bytes received
    ///@param shouldFill Whether to wait for the buffer to fill up completely
    int receiveBase(void* buffer, int bufferLength, bool shouldFill);

    ///@param n Amount of bytes to receive
    std::string receiveString(size_t);
    std::string receiveString();

public:
    void Send(const void* data, int dataLength) const;

    template <typename Container>
    inline void Send(const Container& c) const { Send(std::data(c), std::size(c)); }


    ///Receives exactly bufferLength bytes from the socket
    ///@return Bytes received, greater than zero
    // int Receive(void* buffer, int bufferLength);

    ///Receives exactly bufferLength bytes from the socket<br>
    ///Returns 0 instead of throwing SockGracefulDisconnect
    // int ReceiveNX(void* buffer, int bufferLength);

    inline std::string Receive() { return receiveString(); }
    inline std::string Receive(size_t n) { return receiveString(n); }

    ///Returns an empty string instead of throwing SockGracefulDisconnect
    // std::string ReceiveNX();

    ///Disconnect and receive remaining data
    // std::string DisconnectGet();

    inline bool HasData() { return sock.Readable(); }
};


class SockClient : public SockConnection {
public:
    bool Connect(const char* host, uint16_t port);
};


class ConnectedClient : public SockConnection {
public:
    Host host;
protected:
    ConnectedClient(SockPlatform::socket_t&& in_sock, const sockaddr_in& clientHostInfo)
        : SockConnection(std::move(in_sock)), host(clientHostInfo) { }
    friend class SockServer;
};

class SockServer : public SockStreamBase {
public:
    ///@param backlog Max length of the pending connections queue
    void Start(uint16_t port, int backlog = 256);
    ConnectedClient Accept();
    inline bool HasClients() { return sock.Readable(); }
};


class SockDisconnect : public SockError {
public:
    template <typename Func>
    explicit SockDisconnect(Func func, int wsaerror) : SockError("Lost connection to remote host", func, wsaerror) { }
protected:
    using SockError::SockError;
};
