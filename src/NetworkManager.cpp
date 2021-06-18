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

void NetworkManager::Run(GameManager* pGameManager) {
	mSock->Bind(mAddr);
	mSock->Listen(15);
	for (int i = 0; i < (int)mSysInfo.dwNumberOfProcessors; ++i) {
		std::thread(PacketWorker, mIOCP, this).detach();
	}
	std::thread(AcceptWorker, mSock, mIOCP, this).detach();
	mGameManager = pGameManager;
}

void NetworkManager::PacketWorker(IOCPPtr pIOCP, NetworkManager* pNetworkManager) {
	LPIOCP_KEY_DATA pKeyData;
	LPEXT_OVERLAPPED pExtOver;

	while (1) {
		unsigned ioBytes = pIOCP->GetCompletion(pKeyData, pExtOver);
		switch (pExtOver->mode) {
		case MODE_RECV:
			if (ioBytes == 0) {
				// TODO: 클라이언트 연결 종료 처리, 현재 접속 중인 유저 수 감소
				pNetworkManager->DecreaseClntCount();
			}
			else {
				pNetworkManager->ConstructPacket(pKeyData->clntAddr, (uint8_t*)pExtOver->buffer, ioBytes);
				pKeyData->pClntSock->Receive();
			}
			break;
		case MODE_SEND:
			break;
		}
	}
}

void NetworkManager::AcceptWorker(TCPSocketPtr pSock, IOCPPtr pIOCP, NetworkManager* pNetworkManager) {
	while (1) {
		SocketAddress clntAddr;
		TCPSocketPtr pClntSock(pSock->Accept(clntAddr));
		pClntSock->SetAddr(clntAddr);

		pNetworkManager->RegisterClient(pClntSock, clntAddr);
		pIOCP->ConnectSockToIOCP(pClntSock, clntAddr);

		pClntSock->Receive();
	}
}

void NetworkManager::ConstructPacket(SocketAddress clntAddr, uint8_t* ioBuffer, int remainingBytes) {
	ClientCtxPtr pCc = mAddrToCcpMap[clntAddr];
	while (remainingBytes > 0) {
		int recvdBytesCount = pCc->GetRecvdBytesCount();
		if (recvdBytesCount < sizeof(PACKET_SIZE)) {
			/* 패킷 크기 정보를 모두 읽을 수 있음 */
			if (sizeof(PACKET_SIZE) <= recvdBytesCount + remainingBytes) {
				pCc->RecvPacket(ioBuffer, sizeof(PACKET_SIZE) - recvdBytesCount, false);
				ioBuffer += sizeof(PACKET_SIZE) - recvdBytesCount;
				remainingBytes -= sizeof(PACKET_SIZE) - recvdBytesCount;
			}
			/* 패킷 크기 정보를 모두 읽을 수 없음 */
			else {
				pCc->RecvPacket(ioBuffer, remainingBytes, false);
				break;
			}
		}
		recvdBytesCount = pCc->GetRecvdBytesCount();
		const int packetSize = pCc->GetPacketSize<PACKET_SIZE>();
		/* 패킷을 완성하여 처리 가능 */
		if (packetSize <= recvdBytesCount + remainingBytes) {
			pCc->RecvPacket(ioBuffer, packetSize - recvdBytesCount, true);
			ProcessPacket(pCc);
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

void NetworkManager::ProcessPacket(ClientCtxPtr pCc) {
	switch (pCc->GetPacketType<PACKET_SIZE, PACKET_TYPE>()) {
	case CS_REQ_CONNECT:
		HandleRequestConnect(pCc);
		break;
	}
}

void NetworkManager::SendMatchSuccessPacket(
	ClientCtxPtr clientA,
	ClientCtxPtr clientB,
	uint8_t chA,
	std::string nameA,
	uint8_t chB,
	std::string nameB)
{
	uint8_t type = SC_MATCH_SUCCESS;

	uint16_t sizeA = 2;
	sizeA += 1;				// 패킷 타입
	sizeA += 1;				// B의 캐릭터
	sizeA += 1;				// B의 이름 길이 정보
	sizeA += nameB.size();	// B의 이름 길이
	uint8_t sizeNameB = nameB.size();

	OutputBitStream obsA;
	obsA.WriteBytes(reinterpret_cast<void*>(&sizeA), 2);
	obsA.WriteBytes(reinterpret_cast<void*>(&chB), 1);
	obsA.WriteBytes(reinterpret_cast<void*>(&sizeNameB), 1);
	obsA.WriteBytes(reinterpret_cast<void*>(const_cast<char*>(nameB.c_str())), sizeNameB);

	uint16_t sizeB = 2;
	sizeB += 1;				// 패킷 타입
	sizeB += 1;				// A의 캐릭터
	sizeB += 1;				// A의 이름 길이 정보
	sizeB += nameA.size();	// A의 이름 길이
	uint8_t sizeNameA = nameA.size();

	OutputBitStream obsB;
	obsB.WriteBytes(reinterpret_cast<void*>(&sizeB), 2);
	obsB.WriteBytes(reinterpret_cast<void*>(&chA), 1);
	obsB.WriteBytes(reinterpret_cast<void*>(&sizeNameA), 1);
	obsB.WriteBytes(reinterpret_cast<void*>(const_cast<char*>(nameA.c_str())), sizeNameA);

	clientA->SendPacket(obsA.GetBufferPtr(), obsA.GetByteLength());
	clientB->SendPacket(obsB.GetBufferPtr(), obsB.GetByteLength());
}

void NetworkManager::SendRequestDeployPacket(ClientCtxPtr clientA, ClientCtxPtr clientB) {
	uint16_t size = 4;
	uint8_t type = SC_REQ_DEPLOY;
	uint8_t timeLimit = DEPLOY_TIME;

	OutputBitStream obs;
	obs.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obs.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obs.WriteBytes(reinterpret_cast<void*>(&timeLimit), 1);

	clientA->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
	clientB->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
}

void NetworkManager::HandleRequestConnect(ClientCtxPtr pCc) {
	const uint8_t* payload = pCc->GetPayload<PACKET_SIZE, PACKET_TYPE>();
	const uint32_t payloadSize = pCc->GetPayloadSizeInBits<PACKET_SIZE, PACKET_TYPE>();
	
	InputBitStream ibs(payload, payloadSize);

	uint8_t mapCode;
	uint8_t character;
	uint8_t nameLen;
	uint8_t name[20] = { 0 };

	ibs.ReadBytes(reinterpret_cast<void*>(&mapCode), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&character), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&nameLen), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&name), nameLen);

	mGameManager->EnqueueWaitingQ(pCc, std::string(reinterpret_cast<char*>(name)), character, mapCode);

	OutputBitStream obs;

	uint16_t size = 4;
	uint8_t type = SC_RES_CONNECT;
	uint8_t resCode = 0;

	obs.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obs.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obs.WriteBytes(reinterpret_cast<void*>(&resCode), 1);

	pCc->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
}