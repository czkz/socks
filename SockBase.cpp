#include "SockBase.h"

SockBase::SockResource::SockResource(int af, int type, int protocol) {
	WSADATA wsadata;
	if (WSAStartup(0x202, &wsadata)) {
		throw SockError("Socks WSAStartup() failed", &WSAStartup, WSAGetLastError());
	}

	sock = socket (af, type, protocol);
	if (sock == INVALID_SOCKET) {
		throw SockError("Socks socket() failed", &socket, WSAGetLastError());
	}
	FD_ZERO(&thisSet);
}

SockBase::SockResource::SockResource(SOCKET&& socket) {
	WSADATA wsadata;
	if (WSAStartup(0x202, &wsadata)) {
		throw SockError("Socks WSAStartup() failed", &WSAStartup, WSAGetLastError());
	}

	sock = socket;
	socket = INVALID_SOCKET;
	FD_ZERO(&thisSet);
}

SockBase::SockResource::~SockResource() {
	WSACleanup();
	if (sock != INVALID_SOCKET) {
		closesocket(sock);
	}
}

bool SockBase::SockResource::Readable() {
    static constexpr timeval tv = {0, 0};
	FD_SET(sock, &thisSet);
    int ret = select(0, &thisSet, 0, 0, const_cast<timeval*>(&tv));
    if (ret == SOCKET_ERROR) {
        throw SockError("Socks select() failed", &select, WSAGetLastError());
    }
    else {
        return ret;
    }
}

std::string Host::GetHostname() const {
    if (!hostInfo) { return ""; }
    HOSTENT* hst = gethostbyaddr ((const char*) &hostInfo->sin_addr.s_addr, 4, AF_INET);
    if (hst == nullptr) {
        throw SockError("Sock Host gethostbyaddr() error", &gethostbyaddr, WSAGetLastError());
    }
    return hst->h_name;
}

Host::Host(const sockaddr_in& hostInfo) noexcept
    : hostInfo(std::make_optional<sockaddr_in>(hostInfo)),
      ip(inet_ntoa(hostInfo.sin_addr)),
      port(ntohs(hostInfo.sin_port)) { }

Host::Host(std::optional<sockaddr_in>&& hostInfo) noexcept
    : hostInfo(std::move(hostInfo)),
      ip(hostInfo ? inet_ntoa(hostInfo->sin_addr) : ""),
      port(hostInfo ? ntohs(hostInfo->sin_port) : 0) { }

Host::Host(const char* host, uint16_t port)
    : Host(GetSockaddr(host, port)) { }

std::optional<sockaddr_in> Host::GetSockaddr(const char* host, uint16_t port) {
    sockaddr_in hostInfo;
	hostInfo.sin_family = AF_INET;
	hostInfo.sin_port = htons(port);

	unsigned long asIP = inet_addr (host);
	if (asIP != INADDR_NONE) {
		hostInfo.sin_addr.s_addr = asIP;
	}
	else {
		HOSTENT* h;
		h = gethostbyname(host);
		if (!h) {
			int err = WSAGetLastError();
			if (err == WSANO_DATA) { return std::nullopt; }
			else { throw SockError("Sock failed to interpret host address", &gethostbyname, err); }
		}
		else {
			hostInfo.sin_addr.s_addr = **((unsigned long**) h->h_addr_list);
		}
	}
	return std::make_optional<sockaddr_in>(hostInfo);
}
