#include "Host.h"
#include "SockError.h"


std::string Host::GetHostname() const {
    if (!hostInfo) { return ""; }
    SockPlatform::hostent_t* hst = gethostbyaddr ((const char*) &hostInfo->sin_addr.s_addr,
                                                 4, AF_INET);
    if (hst == nullptr) {
        throw SockError("Sock Host gethostbyaddr() error",
                        &gethostbyaddr, SockPlatform::get_errno());
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
    : Host(getSockaddr(host, port)) { }

std::optional<sockaddr_in> Host::getSockaddr(const char* host, uint16_t port) {
    sockaddr_in hostInfo;
    hostInfo.sin_family = AF_INET;
    hostInfo.sin_port = htons(port);

    unsigned long asIP = inet_addr (host);
    if (asIP != INADDR_NONE) {
        hostInfo.sin_addr.s_addr = asIP;
    } else {
        SockPlatform::hostent_t* h;
        h = gethostbyname(host);
        if (h == nullptr) {
            int err = SockPlatform::get_errno();
            if (err == SockPlatform::error_codes::no_data) {
                return std::nullopt;
            } else {
                throw SockError("Sock failed to interpret host address", &gethostbyname, err);
            }
        } else {
            hostInfo.sin_addr.s_addr = **((unsigned long**) h->h_addr_list);
        }
    }
    return std::make_optional<sockaddr_in>(hostInfo);
}
