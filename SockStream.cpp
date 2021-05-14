#include "SockStream.h"

#ifdef _WIN32
    void SockStreamBase::Disconnect() const { shutdown(sock.value, SD_SEND); }
#elif __linux__
    void SockStreamBase::Disconnect() const { shutdown(sock.value, SHUT_WR); }
#endif

void SockConnection::Send(const void* data, int dataSize) const {
    const int res = send(sock.value, (const char*) data, dataSize, 0);
    if (res == -1) {
        int err = SockDefines::get_errno();
        switch (err) {
        case SockDefines::error_codes::ewouldblock:
            return;
        case SockDefines::error_codes::ehostunreach:
        case SockDefines::error_codes::econnreset:
        case SockDefines::error_codes::etimedout:
            throw SockDisconnect(&send, err);
        default:
            throw SockError("Sock send() failed", &send, err);
        }
    }
}

int SockConnection::ReceiveBase(void* buffer, int bufferLength, bool shouldFill) {
    const int res = recv(sock.value, (char*) buffer, bufferLength, shouldFill ? MSG_WAITALL : 0);
    if (res == 0) {
        throw SockGracefulDisconnect(*this);
    } else if (res < 0) {
        int err = SockDefines::get_errno();
        switch (err) {
        case SockDefines::error_codes::ewouldblock:
            return 0;
        case SockDefines::error_codes::enetreset:
        case SockDefines::error_codes::etimedout:
        case SockDefines::error_codes::econnreset:
            throw SockDisconnect(&recv, err);
        default:
            throw SockError("Sock recv() failed", &recv, err);
        }
    } else {
        return res;
    }
}

int SockConnection::ReceiveNX(void* buffer, int bufferLength) {
    try { return ReceiveBase(buffer, bufferLength, true); }
    catch (const SockGracefulDisconnect& e) {
        return 0;
    }
}

std::string SockConnection::ReceiveString(size_t n) {
    std::string s;
    s.reserve(n);
    constexpr unsigned int buflen = 256 * 1024;
    char buf[buflen];
    int rl;
    for (size_t r = n; r; r -= rl) {
        rl = Receive(buf, std::min<size_t>(buflen, r));
        s.append(buf, rl);
    }
    return s;
}

std::string SockConnection::ReceiveString() {
    std::string s;
    constexpr unsigned int buflen = 256 * 1024;
    char buf[buflen];

    do {
        int recvlen = ReceiveBase(buf, buflen, false);
        s.append(buf, recvlen);
    } while (HasData());

    return s;
}

std::string SockConnection::ReceiveNX() {
    try { return ReceiveString(); }
    catch (const SockGracefulDisconnect&) {
        return "";
    }
}

std::string SockConnection::DisconnectGet() {
    Disconnect();
    std::string s, t;
    do {
        std::string t = ReceiveNX();
        s.append(t);
    } while (!t.empty());
    return s;
}


bool SockClient::Connect(const char* host, uint16_t port) {
    Host m_host {host, port};
    if (!m_host.hostInfo) { return false; }

    if (connect(sock.value, (sockaddr*) &m_host.hostInfo.value(), sizeof(sockaddr_in)) != 0) {
        int err = SockDefines::get_errno();
        switch (err) {
        case SockDefines::error_codes::econnrefused:
        case SockDefines::error_codes::enetunreach:
        case SockDefines::error_codes::etimedout:
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
        throw SockError("Sock bind() error", &bind, SockDefines::get_errno());
    }
    if (listen(sock.value, backlog)) {
        throw SockError("Sock listen() error", &listen, SockDefines::get_errno());
    }
}

ConnectedClient SockServer::Accept() {
    SockDefines::socket_t client_socket;
    sockaddr_in clientInfo;
    SockDefines::sockaddr_len_t client_addr_size = sizeof(clientInfo);

    client_socket = accept(sock.value, (sockaddr*) &clientInfo, &client_addr_size);
    if (client_socket == SockDefines::null_socket) {
        throw SockError("Sock accept() error", &accept, SockDefines::get_errno());
    }
    return ConnectedClient(std::move(client_socket), clientInfo);
}
