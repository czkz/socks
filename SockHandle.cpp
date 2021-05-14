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
    FD_ZERO(&thisSet);
}

SockHandle::SockHandle(SockPlatform::socket_t&& socket) {
    this->value = socket;
    socket = SockPlatform::null_socket;
    FD_ZERO(&thisSet);
}

SockHandle::~SockHandle() {
    if (this->value != SockPlatform::null_socket) {
        SockPlatform::close_fn(this->value);
    }
}

bool SockHandle::Readable() {
    static constexpr timeval tv = {0, 0};
    FD_SET(this->value, &thisSet);
    int ret = select(0, &thisSet, 0, 0, const_cast<timeval*>(&tv));
    if (ret == SockPlatform::null_socket) {
        throw SockError("Socks select() failed", &select, SockPlatform::get_errno());
    } else {
        return ret;
    }
}
