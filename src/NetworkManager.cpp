#include "../include/NetworkManager.h"

NetworkManager* NetworkManager::mInstance = nullptr;

NetworkManager::NetworkManager() :
	mServIP(SERVER_IP),
	mServPort(SERVER_PORT),
	mMaxClnts(MAX_CLIENTS),
	mClntCounts(0)
{
	GetSystemInfo(&mSysInfo);
	mClntCounts = 0;
	mSock = SocketUtil::CreateTCPSocket();
	mIOCP = SocketUtil::CreateIOCP();
	mAddr = SocketAddress(mServIP, mServPort);
}

void NetworkManager::Run() {
	mSock->Bind(mAddr);
	mSock->Listen(15);
	for (int i = 0; i < (int)mSysInfo.dwNumberOfProcessors; ++i) {
		std::thread(PacketWorker, mIOCP, this).detach();
	}
	std::thread(AcceptWorker, mSock, mIOCP, this).detach();
}

void NetworkManager::PacketWorker(IOCPPtr pIOCP, NetworkManager* pNm) {
	LPIOCP_KEY_DATA pKeyData;
	LPEXT_OVERLAPPED pExtOver;

	while (1) {
		unsigned ioBytes = pIOCP->GetCompletion(pKeyData, pExtOver);
		switch (ioBytes) {
		case MODE_RECV:
			if (ioBytes == 0) {
				// TODO: 클라이언트 연결 종료 처리, 현재 접속 중인 유저 수 감소
				pNm->DecreaseClntCount();
			}
			else {
				pNm->ConstructPacket(pKeyData->clntAddr, (uint8_t*)pExtOver->buffer, ioBytes);
				pKeyData->pClntSock->Receive();
			}
			break;
		case MODE_SEND:
			break;
		}
	}
}

void NetworkManager::AcceptWorker(TCPSocketPtr pSock, IOCPPtr pIOCP, NetworkManager* pNm) {
	while (1) {
		SocketAddress clntAddr;
		TCPSocketPtr pClntSock(pSock->Accept(clntAddr));
		pClntSock->SetAddr(clntAddr);
		pIOCP->ConnectSockToIOCP(pClntSock, clntAddr);
		pClntSock->Receive();
	}
}

void NetworkManager::ConstructPacket(SocketAddress clntAddr, uint8_t* ioBuffer, int remainingBytes) {
	ClientCtxPtr pCc = mAddrToCcpMap[clntAddr];

	while (remainingBytes > 0) {
		const int recvdSizeBytesCount = pCc->GetRecvdSizeBytesCount();
		if (recvdSizeBytesCount < sizeof(PACKET_SIZE)) {
			/* 패킷 크기 정보를 모두 읽을 수 있음 */
			if (sizeof(PACKET_SIZE) <= recvdSizeBytesCount + remainingBytes) {
				pCc->RecvSizeBytes(ioBuffer, sizeof(PACKET_SIZE) - recvdSizeBytesCount);
				ioBuffer += sizeof(PACKET_SIZE) - recvdSizeBytesCount;
				remainingBytes -= sizeof(PACKET_SIZE) - recvdSizeBytesCount;
			}
			/* 패킷 크기 정보를 모두 읽을 수 없음 */
			else {
				pCc->RecvSizeBytes(ioBuffer, remainingBytes);
				break;
			}
		}
		const int recvdBytesCount = pCc->GetRecvdBytesCount();
		int packetSize = pCc->GetPacketSize();
		/* 패킷을 완성하여 처리 가능 */
		if (packetSize <= pCc->GetRecvdBytesCount() + remainingBytes) {
			pCc->RecvPacket(ioBuffer, packetSize - recvdBytesCount, true);
			ioBuffer += packetSize - recvdBytesCount;
			remainingBytes -= packetSize - recvdBytesCount;
		}
		/* 패킷을 완성할 수 없으므로, 클라이언트 컨텍스트에 백업 */
		else {
			pCc->RecvPacket(ioBuffer, remainingBytes, false);
			break;
		}
	}
}