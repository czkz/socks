#pragma once
#include "SockBase.h"


class SockStreamBase : public SockBase {
protected:
    SockStreamBase() : SockBase(AF_INET, SOCK_STREAM, IPPROTO_TCP) { }
    SockStreamBase(SOCKET&& sock) : SockBase(std::move(sock)) { }
public:
	///Initialize graceful disconnect sequence.
	inline void Disconnect() const { shutdown(sockrc->sock, SD_SEND); }
};

class SockConnection : public SockStreamBase {
protected:
    using SockStreamBase::SockStreamBase;
	///@return Bytes received
	///@param shouldFill Whether to wait for the buffer to fill up completely
	int ReceiveBase(void* buffer, int bufferLength, bool shouldFill);
	std::string ReceiveString();
	///@param n Amount of bytes to receive
	std::string ReceiveString(size_t);
public:
    void Send(const void* data, int dataLength) const;
	template <typename Container> inline void Send(const Container& c) const { Send(c.data(), c.size()); }


	///Receives exactly bufferLength bytes from the socket
	///@return Bytes received, greater than zero
	inline int Receive(void* buffer, int bufferLength) { return ReceiveBase(buffer, bufferLength, true); }

	///Receives exactly bufferLength bytes from the socket<br>
	///Returns 0 instead of throwing SockGracefulDisconnect
	int ReceiveNX(void* buffer, int bufferLength);

	inline std::string Receive() { return ReceiveString(); }
	inline std::string Receive(size_t n) { return ReceiveString(n); }

	///Returns an empty string instead of throwing SockGracefulDisconnect
	std::string ReceiveNX();

	///Disconnect and receive remaining data
	std::string DisconnectGet();

	inline bool HasData() { return sockrc->Readable(); }
};


class SockClient : public SockConnection {
public:
	bool Connect(const char* host, uint16_t port);
};


class ConnectedClient : public SockConnection {
public:
	Host host;
protected:
	ConnectedClient(SOCKET&& in_sock, const sockaddr_in& clientHostInfo)
        : SockConnection(std::move(in_sock)), host(clientHostInfo) { }
	friend class SockServer;
};

class SockServer : public SockStreamBase {
public:
	///@param backlog Max length of the pending connections queue
	void Start(uint16_t port, int backlog = 256);
	ConnectedClient Accept();
	inline bool HasClients() { return sockrc->Readable(); }
};


class SockDisconnect : public SockError {
public:
    template <typename function>
    explicit SockDisconnect(function func, int wsaerror) : SockError("Lost connection to remote host", func, wsaerror) { };
protected:
    using SockError::SockError;
};

class SockGracefulDisconnect : public SockDisconnect {
public:
    const SockConnection& sock;
    explicit SockGracefulDisconnect(const SockConnection& sock) : SockDisconnect("Remote host disconnected", &recv, 0), sock(sock) { };
};
