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

	EXT_OVERLAPPED mRecvExtOver;		// overlapped ����ü Ȯ��

	uint32_t mPacketSize;				// ��Ŷ ũ��
	uint32_t mRecvdSizeBytesCount;		// ��Ŷ ũ�⸦ ��ŭ �о�鿴�°�

	uint32_t mRecvdBytesCount;			// �о���� ����Ʈ ��
	uint8_t mPacketBuf[MAX_BUF_SIZE];	// ��Ŷ�� ���� ����
	
	bool mIsValid;						// �̸�, ĳ����, ���� �����ߴ°�

	// TODO: ���� ������ ������ �߰�
};

typedef std::shared_ptr<ClientCtx> ClientCtxPtr;

#endif