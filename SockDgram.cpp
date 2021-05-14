#include "SockDgram.h"
#include "SockError.h"

void SockDgram::Send(const std::string& data, const Host& host) {
    if (!host.hostInfo) { return; }
    int ret = sendto(sock.value, data.data(), data.size(), 0, (const sockaddr*) &host.hostInfo.value(), sizeof(sockaddr_in));
    if (ret == -1) {
        throw SockError("Sock sendto() error", &sendto, SockPlatform::get_errno());
    }
}

void SockDgram::Bind(uint16_t port) {
    sockaddr_in hostInfo;
    hostInfo.sin_family = AF_INET;
    hostInfo.sin_addr.s_addr = INADDR_ANY;
    hostInfo.sin_port = htons(port);

    if (bind(sock.value, (sockaddr*) &hostInfo, sizeof hostInfo) == -1) {
        throw SockError("Sock bind() error", &bind, SockPlatform::get_errno());
    }
}

Packet SockDgram::Receive() {
    constexpr unsigned int buflen = 65536;
    char buf[buflen];

    sockaddr_in hostInfo;
    SockPlatform::sockaddr_len_t hostInfoLen = sizeof hostInfo;

    int recvlen = recvfrom(sock.value, buf, buflen, 0, (sockaddr*) &hostInfo, &hostInfoLen);
    if (recvlen == -1) {
        int err = SockPlatform::get_errno();
        if (err == SockPlatform::error_codes::econnreset) {
            return Packet{std::nullopt, Host{hostInfo}};
        }
        else {
            throw SockError("Sock recvfrom() error", &recvfrom, err);
        }
    }

    return Packet{std::string{buf, (std::string::size_type) recvlen}, Host{hostInfo}};
}


void SockDgramConn::Connect(const Host& host) {
    if (!host.hostInfo) { return; }
    int ret = connect(sock.value, (const sockaddr*) &host.hostInfo.value(), sizeof(host.hostInfo.value()));
    if (ret == -1) {
        throw SockError("Sock connect() error", &connect, SockPlatform::get_errno());
    }
}

void SockDgramConn::Send(const std::string& data)
{
    int ret = send(sock.value, data.data(), data.size(), 0);
    if (ret == -1) {
        throw SockError("Sock send() error", &send, SockPlatform::get_errno());
    }
}

std::optional<std::string> SockDgramConn::Receive()
{
    constexpr unsigned int buflen = 65536;
    char buf[buflen];

    int recvlen = recv(sock.value, buf, buflen, 0);
    if (recvlen == -1) {
        int err = SockPlatform::get_errno();
        if (err == SockPlatform::error_codes::econnreset) {
            return std::nullopt;
        }
        else {
            throw SockError("Sock recv() error", &recv, err);
        }
    }

    return std::string{buf, (std::string::size_type) recvlen};
}

