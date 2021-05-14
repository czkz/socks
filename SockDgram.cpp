#include "SocksDgram.h"

void SockDgram::Send(const std::string& data, const Host& host) {
    if (!host.hostInfo) { return; }
    int ret = sendto(sockrc->sock, data.data(), data.size(), 0, (const sockaddr*) &host.hostInfo.value(), sizeof(sockaddr_in));
    if (ret == SOCKET_ERROR) {
        throw SockError("Sock sendto() error", &sendto, WSAGetLastError());
    }
}

void SockDgram::Bind(uint16_t port) {
    sockaddr_in hostInfo;
    hostInfo.sin_family = AF_INET;
    hostInfo.sin_addr.s_addr = INADDR_ANY;
    hostInfo.sin_port = htons(port);

    if (bind(sockrc->sock, (sockaddr*) &hostInfo, sizeof hostInfo) == SOCKET_ERROR) {
		throw SockError("Sock bind() error", &bind, WSAGetLastError());
	}
}

Packet SockDgram::Receive() {
	constexpr unsigned int buflen = 65536;
	char buf[buflen];

	sockaddr_in hostInfo;
	int hostInfoLen = sizeof hostInfo;

	int recvlen = recvfrom(sockrc->sock, buf, buflen, 0, (sockaddr*) &hostInfo, &hostInfoLen);
	if (recvlen == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAECONNRESET) {
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
    int ret = connect(sockrc->sock, (const sockaddr*) &host.hostInfo.value(), sizeof(host.hostInfo.value()));
    if (ret == SOCKET_ERROR) {
        throw SockError("Sock connect() error", &connect, WSAGetLastError());
    }
}

void SockDgramConn::Send(const std::string& data)
{
    int ret = send(sockrc->sock, data.data(), data.size(), 0);
    if (ret == SOCKET_ERROR) {
        throw SockError("Sock send() error", &send, WSAGetLastError());
    }
}

std::optional<std::string> SockDgramConn::Receive()
{
    constexpr unsigned int buflen = 65536;
	char buf[buflen];

	int recvlen = recv(sockrc->sock, buf, buflen, 0);
	if (recvlen == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAECONNRESET) {
            return std::nullopt;
        }
        else {
            throw SockError("Sock recv() error", &recv, err);
        }
    }

	return std::string{buf, (std::string::size_type) recvlen};
}

