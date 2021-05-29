#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

#include <WinSock2.h>
#include <memory>

#include "SocketAddress.h"
#include "../include/Config.h"

class NetworkManager;

typedef struct {
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[MAX_BUF_SIZE];
	int mode;
} EXT_OVERLAPPED, * LPEXT_OVERLAPPED;

class TCPSocket {
public:
	~TCPSocket() { delete pRecvExtOver; closesocket(mSock); }
	int Connect(const SocketAddress& inAddress);
	int Bind(const SocketAddress& inToAddress);
	int Listen(int inBackLog);
	std::shared_ptr<TCPSocket> Accept(SocketAddress& inFromAddress);
	int Send(const void* inData, int inLen);
	int Receive();
	void SetAddr(SocketAddress sa) { mAddr = sa; }

	SOCKET GetSocket() { return mSock; }

private:
	friend class SocketUtil;
	friend class ClientData;
	TCPSocket(SOCKET inSocket) : mSock(inSocket) { pRecvExtOver = new EXT_OVERLAPPED; }
	SOCKET mSock;
	SocketAddress mAddr;

	LPEXT_OVERLAPPED pRecvExtOver;
};

typedef std::shared_ptr<TCPSocket> TCPSocketPtr;

#endif