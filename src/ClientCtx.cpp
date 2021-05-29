#include "../include/ClientCtx.h"

ClientCtx::ClientCtx(TCPSocketPtr sock, SocketAddress sockAddr) : 
	mSock(sock),
	mAddr(sockAddr),
	mPacketSize(0),
	mRecvdSizeBytesCount(0),
	mRecvdBytesCount(0),
	mIsValid(false) { }

void ClientCtx::RecvSizeBytes(uint8_t* ioBuffer, int count) {
	memcpy(&mPacketSize + mRecvdSizeBytesCount, ioBuffer, count);
	mRecvdSizeBytesCount += count;
}

void ClientCtx::RecvPacket(uint8_t* ioBuffer, int count, bool isDone) {
	memcpy(mPacketBuf + mRecvdBytesCount, ioBuffer, count);
	mRecvdBytesCount += count;
	if (isDone) mRecvdSizeBytesCount = 0;
}