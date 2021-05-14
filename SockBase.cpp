#include "SockBase.h"


SockHandle::SockHandle(SockHandle&& other) {
    this->value = other.value;
    other.value = SockDefines::null_socket;
}

SockHandle& SockHandle::operator=(SockHandle&& other) {
    this->value = other.value;
    other.value = SockDefines::null_socket;
    return *this;
}

SockHandle::SockHandle(int af, int type, int protocol) {
	this->value = socket(af, type, protocol);
	if (this->value == SockDefines::null_socket) {
		throw SockError("Socks socket() failed", &socket, SockDefines::get_errno());
	}
	FD_ZERO(&thisSet);
}

SockHandle::SockHandle(SockDefines::socket_t&& socket) {
	this->value = socket;
	socket = SockDefines::null_socket;
	FD_ZERO(&thisSet);
}

#ifdef _WIN32
SockHandle::~SockHandle() {
	if (this->value != SockDefines::null_socket) {
		closesocket(this->value);
	}
}
#elif __linux__
SockHandle::~SockHandle() {
	if (this->value != SockDefines::null_socket) {
		close(this->value);
	}
}
#endif

bool SockHandle::Readable() {
    static constexpr timeval tv = {0, 0};
	FD_SET(this->value, &thisSet);
    int ret = select(0, &thisSet, 0, 0, const_cast<timeval*>(&tv));
    if (ret == SockDefines::null_socket) {
        throw SockError("Socks select() failed", &select, SockDefines::get_errno());
    }
    else {
        return ret;
    }
}



std::string Host::GetHostname() const {
    if (!hostInfo) { return ""; }
    SockDefines::hostent_t* hst = gethostbyaddr ((const char*) &hostInfo->sin_addr.s_addr, 4, AF_INET);
    if (hst == nullptr) {
        throw SockError("Sock Host gethostbyaddr() error", &gethostbyaddr, SockDefines::get_errno());
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
        SockDefines::hostent_t* h;
        h = gethostbyname(host);
        if (!h) {
            int err = SockDefines::get_errno();
            if (err == SockDefines::error_codes::no_data) { return std::nullopt; }
            else { throw SockError("Sock failed to interpret host address", &gethostbyname, err); }
        }
        else {
            hostInfo.sin_addr.s_addr = **((unsigned long**) h->h_addr_list);
        }
    }
    return std::make_optional<sockaddr_in>(hostInfo);
}
