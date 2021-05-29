#include "TCPSocket.h"
#include "SocketUtil.h"

int TCPSocket::Connect(const SocketAddress& inAddress) {
	int err = connect(mSock, &inAddress.mSockAddr, (int)inAddress.GetSize());
	if (err < 0) {
		SocketUtil::ReportError("TCPSocket::Connect");
		return -SocketUtil::GetLastError();
	}
	return 0;
}

int TCPSocket::Bind(const SocketAddress& inToAddress) {
	int err = bind(mSock, &inToAddress.mSockAddr, (int)inToAddress.GetSize());
	if (err == SOCKET_ERROR) {
		SocketUtil::ReportError("TCPSocket::Bind");
		return -SocketUtil::GetLastError();
	}
	return 0;
}

int TCPSocket::Listen(int inBackLog) {
	int err = listen(mSock, inBackLog);
	if (err == SOCKET_ERROR) {
		SocketUtil::ReportError("TCPSocket::Listen");
		return -SocketUtil::GetLastError();
	}
	return 0;
}

std::shared_ptr<TCPSocket> TCPSocket::Accept(SocketAddress& inFromAddress) {
	socklen_t length = inFromAddress.GetSize();
	SOCKET sock = accept(mSock, &inFromAddress.mSockAddr, &length);
	if (sock == INVALID_SOCKET) {
		SocketUtil::ReportError("TCPSocket::Accept");
		return nullptr;
	}
	return std::shared_ptr<TCPSocket>(new TCPSocket(sock));
}

int TCPSocket::Send(const void* inData, int inLen) {
	DWORD sentBytes = inLen, flags = 0;

	EXT_OVERLAPPED* pSendExtOver = new EXT_OVERLAPPED;
	memset(&(pSendExtOver->overlapped), 0, sizeof(OVERLAPPED));
	memcpy(static_cast<void*>(pSendExtOver->buffer), inData, inLen);
	pSendExtOver->wsaBuf.buf = pSendExtOver->buffer;
	pSendExtOver->wsaBuf.len = inLen;
	pSendExtOver->mode = MODE_SEND;

	int err = WSASend(mSock, &(pSendExtOver->wsaBuf), 1, &sentBytes, flags, &(pSendExtOver->overlapped), NULL);
	if (err == SOCKET_ERROR) {
		int lastErr = SocketUtil::GetLastError();
		if (lastErr != WSA_IO_PENDING) {
			SocketUtil::ReportError("TCPSocket::Send");
			return -lastErr;
		}
	}

	return sentBytes;
}

int TCPSocket::Receive() {
	DWORD recvBytes = 0, flags = 0;

	memset(&(pRecvExtOver->overlapped), 0, sizeof(OVERLAPPED));
	pRecvExtOver->wsaBuf.buf = pRecvExtOver->buffer;
	pRecvExtOver->wsaBuf.len = sizeof(pRecvExtOver->buffer);
	pRecvExtOver->mode = MODE_RECV;

	int err = WSARecv(mSock, &(pRecvExtOver->wsaBuf), 1, &recvBytes, &flags, &(pRecvExtOver->overlapped), NULL);
	if (err == SOCKET_ERROR) {
		int lastErr = SocketUtil::GetLastError();
		if (lastErr != WSA_IO_PENDING) {
			SocketUtil::ReportError("TCPSocket::Receive");
			return -lastErr;
		}
	}

	return recvBytes;
}