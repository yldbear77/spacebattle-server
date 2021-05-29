#ifndef __IOCP_H__
#define __IOCP_H__

#include <Winsock2.h>

#include "TCPSocket.h"

class SocketUtil;

typedef struct {
	TCPSocketPtr pClntSock;
	SocketAddress clntAddr;
	// std::string id;
} IOCP_KEY_DATA, * LPIOCP_KEY_DATA;

class IOCP {
public:
	int32_t ConnectSockToIOCP(TCPSocketPtr clntSock, SocketAddress& sockAddr);
	int32_t GetCompletion(LPIOCP_KEY_DATA& pKeyData, LPEXT_OVERLAPPED& pIOData);

	HANDLE GetIOCP() { return mComPort; }

private:
	friend class SocketUtil;
	IOCP(HANDLE inComPort) : mComPort(inComPort) { }
	HANDLE mComPort;
};

typedef std::shared_ptr<IOCP> IOCPPtr;

#endif