#include "SockPlatform.h"
#include "SockError.h"

#ifdef _WIN32
//////////////////////////////////////////// Windows specific code /////

WSAHandle::WSAHandle() {
    WSADATA wsadata;
    if (WSAStartup(0x202, &wsadata)) {
        throw SockError("Socks WSAStartup() failed", &WSAStartup, WSAGetLastError());
    }
}

WSAHandle::~WSAHandle() {
    WSACleanup();
}

#elif __unix__
//////////////////////////////////////////// Unix specific code ////////

#endif
