#include "../include/ClientCtx.h"

ClientCtx::ClientCtx(TCPSocketPtr sock, SocketAddress sockAddr) : 
	mSock(sock),
	mAddr(sockAddr),
	mRecvdBytesCount(0) { }

void ClientCtx::RecvPacket(uint8_t* ioBuffer, int count, bool isDone) {
	memcpy(mPacketBuf + mRecvdBytesCount, ioBuffer, count);
	mRecvdBytesCount += count;
	if (isDone) mRecvdBytesCount = 0;
}