#include "../include/ClientCtx.h"

ClientCtx::ClientCtx(TCPSocketPtr sock, SocketAddress sockAddr) : 
	mSock(sock),
	mAddr(sockAddr),
	mRecvdBytesCount(0),
	mIsOnline(true) { }

void ClientCtx::RecvPacket(uint8_t* ioBuffer, int count, bool isDone) {
	memcpy(mPacketBuf + mRecvdBytesCount, ioBuffer, count);
	mRecvdBytesCount += count;
	if (isDone) mRecvdBytesCount = 0;
}