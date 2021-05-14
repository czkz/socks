#include "SockStream.h"

void SockConnection::Send(const void* data, int dataSize) const {
	const int res = send(sockrc->sock, (const char*) data, dataSize, 0);
	if (res == SOCKET_ERROR) {
		int err = WSAGetLastError();
		switch (err) {
		case WSAEWOULDBLOCK:
			return;
		case WSAEHOSTUNREACH:
		case WSAECONNRESET:
		case WSAETIMEDOUT:
			throw SockDisconnect(&send, err);
		default:
			throw SockError("Sock send() failed", &send, err);
		}
	}
}

int SockConnection::ReceiveBase(void* buffer, int bufferLength, bool shouldFill) {
	const int res = recv(sockrc->sock, (char*) buffer, bufferLength, shouldFill ? MSG_WAITALL : 0);
	if (res == 0) {
		throw SockGracefulDisconnect(*this);
	}
	else if (res < 0) {
		int err = WSAGetLastError();
		switch (err) {
		case WSAEWOULDBLOCK:
			return 0;
		case WSAENETRESET:
		case WSAETIMEDOUT:
		case WSAECONNRESET:
			throw SockDisconnect(&recv, err);
		default:
			throw SockError("Sock recv() failed", &recv, err);
		}
	}
	else {
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

	if (connect(sockrc->sock, (sockaddr*) &m_host.hostInfo.value(), sizeof(sockaddr_in)) != 0) {
		int err = WSAGetLastError();
		switch (err) {
		case WSAECONNREFUSED:
		case WSAENETUNREACH:
		case WSAETIMEDOUT:
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

	if (bind(sockrc->sock, (sockaddr*) &hostInfo, sizeof hostInfo)) {
		throw SockError("Sock bind() error", &bind, WSAGetLastError());
	}
	if (listen(sockrc->sock, backlog)) {
		throw SockError("Sock listen() error", &listen, WSAGetLastError());
	}
}

ConnectedClient SockServer::Accept() {
	SOCKET client_socket;
	sockaddr_in clientInfo;
	int client_addr_size = sizeof(clientInfo);

	client_socket = accept(sockrc->sock, (sockaddr*) &clientInfo, &client_addr_size);
	if (client_socket == INVALID_SOCKET) {
		throw SockError("Sock accept() error", &accept, WSAGetLastError());
	}
	return ConnectedClient(std::move(client_socket), clientInfo);
}
