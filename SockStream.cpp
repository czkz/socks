#include "SockStream.h"
#include "SockPlatform.h"

void SockStreamBase::Disconnect() const {
    shutdown(sock.value, SockPlatform::shutdown_how::read);
}

void SockConnection::Send(const void* data, int dataSize) const {
    const int res = send(sock.value, (const char*) data, dataSize, 0);
    if (res == -1) {
        int err = SockPlatform::get_errno();
        switch (err) {
        case SockPlatform::error_codes::ewouldblock:
            return;
        case SockPlatform::error_codes::ehostunreach:
        case SockPlatform::error_codes::econnreset:
        case SockPlatform::error_codes::etimedout:
            throw SockDisconnect(&send, err);
        default:
            throw SockError("Sock send() failed", &send, err);
        }
    }
}


// int SockConnection::Receive(void* buffer, int bufferLength) {
//     int ret = ReceiveBase(buffer, bufferLength, true);
//     if (ret == -1) {
//         throw SockDisconnect("Remote host disconnected", &recv, 0);
//     }
//     return ret;
// }

int SockConnection::receiveBase(void* buffer, int bufferLength, bool shouldFill) {
    const int res = recv(sock.value, (char*) buffer, bufferLength, shouldFill ? MSG_WAITALL : 0);
    if (res == 0) {
        throw SockDisconnect("Remote host disconnected", &recv, 0);
    } else if (res < 0) {
        int err = SockPlatform::get_errno();
        switch (err) {
        case SockPlatform::error_codes::ewouldblock:
            return 0;
        case SockPlatform::error_codes::enetreset:
        case SockPlatform::error_codes::etimedout:
        case SockPlatform::error_codes::econnreset:
            throw SockDisconnect(&recv, err);
        default:
            throw SockError("Sock recv() failed", &recv, err);
        }
    } else {
        return res;
    }
}

std::string SockConnection::receiveString(size_t n) {
    std::string s;
    s.resize(n);
    for (size_t i = 0; i < n; ) {
        int res = receiveBase(s.data() + i, s.size() - i, true);
        i += res;
    }
    return s;
}

std::string SockConnection::receiveString() {
    std::string s;
    constexpr unsigned int buflen = 256 * 1024;
    char buf[buflen];

    do {
        int recvlen = receiveBase(buf, buflen, false);
        s.append(buf, recvlen);
    } while (HasData());

    return s;
}

// std::string SockConnection::ReceiveNX() {
//     try { return ReceiveString(); }
//     catch (const SockGracefulDisconnect&) {
//         return "";
//     }
// }

// std::string SockConnection::DisconnectGet() {
//     Disconnect();
//     std::string s, t;
//     do {
//         std::string t = ReceiveNX();
//         s.append(t);
//     } while (!t.empty());
//     return s;
// }


bool SockClient::Connect(const char* host, uint16_t port) {
    Host m_host {host, port};
    if (!m_host.hostInfo) { return false; }

    if (connect(sock.value, (sockaddr*) &m_host.hostInfo.value(), sizeof(sockaddr_in)) != 0) {
        int err = SockPlatform::get_errno();
        switch (err) {
        case SockPlatform::error_codes::econnrefused:
        case SockPlatform::error_codes::enetunreach:
        case SockPlatform::error_codes::etimedout:
            return false;
        }
        throw SockError("Sock connect() error", &connect, err);
    }
    return true;
}


void SockServer::Start(uint16_t port, int backlog) {
    sockaddr_in hostInfo;
    hostInfo.sin_family = AF_INET;
    hostInfo.sin_addr.s_addr = INADDR_ANY;
    hostInfo.sin_port = htons (port);

    if (bind(sock.value, (sockaddr*) &hostInfo, sizeof hostInfo)) {
        throw SockError("Sock bind() error", &bind, SockPlatform::get_errno());
    }
    if (listen(sock.value, backlog)) {
        throw SockError("Sock listen() error", &listen, SockPlatform::get_errno());
    }
}

ConnectedClient SockServer::Accept() {
    SockPlatform::socket_t client_socket;
    sockaddr_in clientInfo;
    SockPlatform::sockaddr_len_t client_addr_size = sizeof(clientInfo);

    client_socket = accept(sock.value, (sockaddr*) &clientInfo, &client_addr_size);
    if (client_socket == SockPlatform::null_socket) {
        throw SockError("Sock accept() error", &accept, SockPlatform::get_errno());
    }
    return ConnectedClient(std::move(client_socket), clientInfo);
}
