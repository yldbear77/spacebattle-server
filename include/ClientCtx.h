#ifndef __CLIENT_CTX_H__
#define __CLIENT_CTX_H__

#include "../net/TCPSocket.h"
#include "../net/SocketAddress.h"

class ClientCtx {
public:
	ClientCtx() = delete;
	ClientCtx(TCPSocketPtr sock, SocketAddress sockAddr);

	const SocketAddress& GetSocketAddr() const { return mAddr; }

	uint32_t GetRecvdBytesCount() const { return mRecvdBytesCount; }
	const uint8_t* GetPacketBuf() const { return mPacketBuf; }

	template <class T>
	uint32_t GetPacketSize() { return *(reinterpret_cast<T*>(&mPacketBuf[0])); }

	template <class T, class U>
	uint32_t GetPacketType() { return *(reinterpret_cast<U*>(&mPacketBuf[sizeof(T)])); }

	template <class T, class U>
	const uint8_t* GetPayload() { return &mPacketBuf[sizeof(T) + sizeof(U)]; }

	template <class T, class U>
	uint32_t GetPayloadSizeInBits() { return (GetPacketSize<T>() - sizeof(T) - sizeof(U)) * 8; }

	void SetRecvdBytesCount(uint32_t bytes) { mRecvdBytesCount = bytes; }

	void RecvPacket(uint8_t* ioBuffer, int count, bool isDone);
	void SendPacket(const uint8_t* ioBuffer, int count) { 
		mSock->Send(reinterpret_cast<void*>(const_cast<uint8_t*>(ioBuffer)), count);
	}

	bool GetOnlineStatus() { return mIsOnline; }
	void UnsetOnlineStatus() { mIsOnline = false; }

private:
	TCPSocketPtr mSock;
	SocketAddress mAddr;

	EXT_OVERLAPPED mRecvExtOver;		// overlapped ����ü Ȯ��

	uint32_t mRecvdBytesCount;			// �о���� ����Ʈ ��
	uint8_t mPacketBuf[MAX_BUF_SIZE];	// ��Ŷ�� ���� ����

	bool mIsOnline;						// ������ �����ǰ� �ִ°�
};

typedef std::shared_ptr<ClientCtx> ClientCtxPtr;

#endif