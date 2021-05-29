#ifndef __CLIENT_CTX_H__
#define __CLIENT_CTX_H__

#include "../net/TCPSocket.h"
#include "../net/SocketAddress.h"

class ClientCtx {
public:
	ClientCtx(TCPSocketPtr sock, SocketAddress sockAddr);

	uint32_t GetPacketSize() { return mPacketSize; }
	uint32_t GetRecvdSizeBytesCount() { return mRecvdSizeBytesCount; }

	uint32_t GetRecvdBytesCount() { return mRecvdBytesCount; }

	void SetRecvdBytesCount(uint32_t bytes) { mRecvdBytesCount = bytes; }

	void RecvSizeBytes(uint8_t* ioBuffer, int count);
	void RecvPacket(uint8_t* ioBuffer, int count, bool isDone);

private:
	TCPSocketPtr mSock;
	SocketAddress mAddr;

	EXT_OVERLAPPED mRecvExtOver;		// overlapped 구조체 확장

	uint32_t mPacketSize;				// 패킷 크기
	uint32_t mRecvdSizeBytesCount;		// 패킷 크기를 얼만큼 읽어들였는가

	uint32_t mRecvdBytesCount;			// 읽어들인 바이트 수
	uint8_t mPacketBuf[MAX_BUF_SIZE];	// 패킷을 담을 버퍼
	
	bool mIsValid;						// 이름, 캐릭터, 맵을 설정했는가

	// TODO: 게임 데이터 포인터 추가
};

typedef std::shared_ptr<ClientCtx> ClientCtxPtr;

#endif