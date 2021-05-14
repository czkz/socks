
#ifdef _WIN32
//////////////////////////////////////////// Windows specific code /////
#include <winsock2.h>
#include <ws2tcpip.h>

namespace SockPlatform {
    using socket_t = SOCKET;
    constexpr socket_t null_socket = INVALID_SOCKET;
    inline int get_errno() { return errno; }
    using hostent_t = HOSTENT;
    using sockaddr_len_t = int;
    static constexpr auto close_fn = &closesocket;

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

    enum shutdown_how {
        read  = SD_RECEIVE,
        write = SD_SEND,
        both  = SD_BOTH,
    };
}

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

#elif __unix__
//////////////////////////////////////////// Unix specific code ////////

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

namespace SockPlatform {
    using socket_t = int;
    constexpr socket_t null_socket = -1;
    inline int get_errno() { return errno; }
    using hostent_t = hostent;
    using sockaddr_len_t = unsigned int;
    static constexpr auto close_fn = &close;

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

    enum shutdown_how {
        read  = SHUT_RD,
        write = SHUT_WR,
        both  = SHUT_RDWR,
    };
}

class WSAHandle { };

#endif
