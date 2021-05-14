#pragma once
#include <optional>
#include <string>
#include "SockHandle.h"
#include "Host.h"

struct Packet {
    Packet(const std::string& data, const Host& host) : data(std::make_optional<std::string>(data)), host(host) { }
    Packet(std::string&& data, const Host& host) : data(std::make_optional<std::string>(std::move(data))), host(host) { }

    std::optional<std::string> data;
    Host host;

private:
    friend class SockDgram;
    Packet(const std::optional<std::string>& data, const Host& host) : data(data), host(host) { }
    Packet(std::optional<std::string>&& data, const Host& host) : data(data), host(host) { }
};

class SockDgram {
protected:
    SockHandle sock;
public:
    SockDgram() : sock(AF_INET, SOCK_DGRAM, IPPROTO_UDP) {}
    void Bind(uint16_t port);
    inline bool HasPackets() { return sock.Readable(); }

    inline void Send(const Packet& packet) { if (packet.data) { return Send(*packet.data, packet.host); } }
    void Send(const std::string& data, const Host& host);

    Packet Receive();
};

class SockDgramConn : private SockDgram {
public:
    using SockDgram::SockDgram;
    using SockDgram::Bind;
    using SockDgram::HasPackets;

    void Connect(const Host& host);

    void Send(const std::string& data);
    std::optional<std::string> Receive();
};
