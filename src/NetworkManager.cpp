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
		switch (pExtOver->mode) {
		case MODE_RECV:
			if (ioBytes == 0) {
				// TODO: Ŭ���̾�Ʈ ���� ���� ó��, ���� ���� ���� ���� �� ����
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

		pNm->RegisterClient(pClntSock, clntAddr);
		pIOCP->ConnectSockToIOCP(pClntSock, clntAddr);

		pClntSock->Receive();
	}
}

void NetworkManager::ConstructPacket(SocketAddress clntAddr, uint8_t* ioBuffer, int remainingBytes) {
	ClientCtxPtr pCc = mAddrToCcpMap[clntAddr];

	while (remainingBytes > 0) {
		const int recvdSizeBytesCount = pCc->GetRecvdSizeBytesCount();
		if (recvdSizeBytesCount < sizeof(PACKET_SIZE)) {
			/* ��Ŷ ũ�� ������ ��� ���� �� ���� */
			if (sizeof(PACKET_SIZE) <= recvdSizeBytesCount + remainingBytes) {
				pCc->RecvSizeBytes(ioBuffer, sizeof(PACKET_SIZE) - recvdSizeBytesCount);
				ioBuffer += sizeof(PACKET_SIZE) - recvdSizeBytesCount;
				remainingBytes -= sizeof(PACKET_SIZE) - recvdSizeBytesCount;
			}
			/* ��Ŷ ũ�� ������ ��� ���� �� ���� */
			else {
				pCc->RecvSizeBytes(ioBuffer, remainingBytes);
				break;
			}
		}
		const int recvdBytesCount = pCc->GetRecvdBytesCount();
		const int packetSize = pCc->GetPacketSize() - sizeof(PACKET_SIZE);
		/* ��Ŷ�� �ϼ��Ͽ� ó�� ���� */
		if (packetSize <= recvdBytesCount + remainingBytes) {
			pCc->RecvPacket(ioBuffer, packetSize - recvdBytesCount, true);
			ProcessPacket(clntAddr);
			ioBuffer += packetSize - recvdBytesCount;
			remainingBytes -= packetSize - recvdBytesCount;
		}
		/* ��Ŷ�� �ϼ��� �� �����Ƿ�, Ŭ���̾�Ʈ ���ؽ�Ʈ�� ��� */
		else {
			pCc->RecvPacket(ioBuffer, remainingBytes, false);
			break;
		}
	}
}

void NetworkManager::ProcessPacket(SocketAddress clntAddr) {
	ClientCtxPtr pCc = mAddrToCcpMap[clntAddr];
	uint32_t packetType = pCc->GetPacketType();
	switch (packetType) {
		// TODO: ��Ŷ�� ���� �б� �ڵ� �ۼ�
	}
}