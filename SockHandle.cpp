#include "SockHandle.h"
#include "SockError.h"


SockHandle::SockHandle(SockHandle&& other) {
    this->value = other.value;
    other.value = SockPlatform::null_socket;
}

SockHandle& SockHandle::operator=(SockHandle&& other) {
    this->value = other.value;
    other.value = SockPlatform::null_socket;
    return *this;
}

SockHandle::SockHandle(int af, int type, int protocol) {
    this->value = socket(af, type, protocol);
    if (this->value == SockPlatform::null_socket) {
        throw SockError("Socks socket() failed", &socket, SockPlatform::get_errno());
    }
}

SockHandle::SockHandle(SockPlatform::socket_t&& socket) {
    this->value = socket;
    socket = SockPlatform::null_socket;
}

SockHandle::~SockHandle() {
    if (this->value != SockPlatform::null_socket) {
        SockPlatform::close_fn(this->value);
    }
}

bool SockHandle::Readable() const {
    fd_set thisSet;
    FD_ZERO(&thisSet);
    FD_SET(this->value, &thisSet);
    timeval tv = {0, 0};
    int ret = select(this->value + 1, &thisSet, 0, 0, &tv);
    if (ret == -1) {
        throw SockError("Socks select() failed", &select, SockPlatform::get_errno());
    } else {
        return ret;
    }
}

bool operator<(const SockHandle& a, const SockHandle& b) {
    return a.value < b.value;
}
