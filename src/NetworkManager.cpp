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
	LOG_NOTIFY("��Ʈ��ũ �Ŵ��� ����");
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
		int ioBytes = pIOCP->GetCompletion(pKeyData, pExtOver);
		if (ioBytes == -ERROR_NETNAME_DELETED) {
			LOG_NOTIFY("Ŭ���̾�Ʈ ����: �����ּ�(%s)", pKeyData->clntAddr.ToString().c_str());
			// TODO: ���� ���� �� �����ϴ� ��쵵 �����Ƿ�, ������ ó�� �ʿ�
			pNetworkManager->DecreaseClntCount();
			pNetworkManager->mAddrToCcpMap[pKeyData->clntAddr]->UnsetOnlineStatus();
			pNetworkManager->mAddrToCcpMap.erase(pKeyData->clntAddr);
			continue;
		}
		switch (pExtOver->mode) {
		case MODE_RECV:
			if (ioBytes == 0) {
				// TODO: Ŭ���̾�Ʈ ���� ���� ó��, ���� ���� ���� ���� �� ����
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

		LOG_NOTIFY("Ŭ���̾�Ʈ ����: �����ּ�(%s)", clntAddr.ToString().c_str());

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
			/* ��Ŷ ũ�� ������ ��� ���� �� ���� */
			if (sizeof(PACKET_SIZE) <= recvdBytesCount + remainingBytes) {
				pCc->RecvPacket(ioBuffer, sizeof(PACKET_SIZE) - recvdBytesCount, false);
				ioBuffer += sizeof(PACKET_SIZE) - recvdBytesCount;
				remainingBytes -= sizeof(PACKET_SIZE) - recvdBytesCount;
			}
			/* ��Ŷ ũ�� ������ ��� ���� �� ���� */
			else {
				pCc->RecvPacket(ioBuffer, remainingBytes, false);
				break;
			}
		}
		recvdBytesCount = pCc->GetRecvdBytesCount();
		const int packetSize = pCc->GetPacketSize<PACKET_SIZE>();
		/* ��Ŷ�� �ϼ��Ͽ� ó�� ���� */
		if (packetSize <= recvdBytesCount + remainingBytes) {
			pCc->RecvPacket(ioBuffer, packetSize - recvdBytesCount, true);
			ProcessPacket(pCc);
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


void NetworkManager::ProcessPacket(ClientCtxPtr pCc) {
	switch (pCc->GetPacketType<PACKET_SIZE, PACKET_TYPE>()) {
	case CS_REQ_CONNECT:
		HandleRequestConnect(pCc);
		break;
	case CS_RES_DEPLOY:
		HandleResponseDeploy(pCc);
		break;
	case CS_REQ_ATTACK:
		HandleRequestAttack(pCc);
		break;
	case CS_REQ_SKILL:
		HandleRequestSkill(pCc);
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
	sizeA += 1;				// ��Ŷ Ÿ��
	sizeA += 1;				// B�� ĳ����
	sizeA += 1;				// B�� �̸� ���� ����
	sizeA += nameB.size();	// B�� �̸� ����
	uint8_t sizeNameB = nameB.size();

	OutputBitStream obsA;
	obsA.WriteBytes(reinterpret_cast<void*>(&sizeA), 2);
	obsA.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obsA.WriteBytes(reinterpret_cast<void*>(&chB), 1);
	obsA.WriteBytes(reinterpret_cast<void*>(&sizeNameB), 1);
	obsA.WriteBytes(reinterpret_cast<void*>(const_cast<char*>(nameB.c_str())), sizeNameB);

	uint16_t sizeB = 2;
	sizeB += 1;				// ��Ŷ Ÿ��
	sizeB += 1;				// A�� ĳ����
	sizeB += 1;				// A�� �̸� ���� ����
	sizeB += nameA.size();	// A�� �̸� ����
	uint8_t sizeNameA = nameA.size();

	OutputBitStream obsB;
	obsB.WriteBytes(reinterpret_cast<void*>(&sizeB), 2);
	obsB.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obsB.WriteBytes(reinterpret_cast<void*>(&chA), 1);
	obsB.WriteBytes(reinterpret_cast<void*>(&sizeNameA), 1);
	obsB.WriteBytes(reinterpret_cast<void*>(const_cast<char*>(nameA.c_str())), sizeNameA);

	LOG_NOTIFY("��Ī ���� ����: �����ּ�(%s)", clientA->GetSocketAddr().ToString().c_str());
	clientA->SendPacket(obsA.GetBufferPtr(), obsA.GetByteLength());

	LOG_NOTIFY("��Ī ���� ����: �����ּ�(%s)", clientB->GetSocketAddr().ToString().c_str());
	clientB->SendPacket(obsB.GetBufferPtr(), obsB.GetByteLength());
}


void NetworkManager::SendRequestDeployPacket(ClientCtxPtr clientA, ClientCtxPtr clientB) {
	uint16_t size = 5;
	uint8_t type = SC_REQ_DEPLOY;
	uint16_t timeLimit = DEPLOY_TIME;

	OutputBitStream obs;
	obs.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obs.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obs.WriteBytes(reinterpret_cast<void*>(&timeLimit), 2);

	LOG_NOTIFY("��ġ ��û ����: �����ּ�(%s)", clientA->GetSocketAddr().ToString().c_str());
	clientA->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());

	LOG_NOTIFY("��ġ ��û ����: �����ּ�(%s)", clientB->GetSocketAddr().ToString().c_str());
	clientB->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
}


void NetworkManager::HandleRequestConnect(ClientCtxPtr pCc) {
	LOG_NOTIFY("ť ���� ��û ����: �����ּ�(%s)", pCc->GetSocketAddr().ToString().c_str());

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

	LOG_NOTIFY("ť ���� ���� ����: �����ּ�(%s)", pCc->GetSocketAddr().ToString().c_str());
	pCc->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
}


void NetworkManager::HandleResponseDeploy(ClientCtxPtr pCc) {
	LOG_NOTIFY("��ġ ���� ����: �����ּ�(%s)", pCc->GetSocketAddr().ToString().c_str());

	const uint8_t* payload = pCc->GetPayload<PACKET_SIZE, PACKET_TYPE>();
	const uint32_t payloadSize = pCc->GetPayloadSizeInBits<PACKET_SIZE, PACKET_TYPE>();

	InputBitStream ibs(payload, payloadSize);

	uint8_t numOfSpacecrafts;
	std::vector<RoomManager::DeployData> deployData;
	
	ibs.ReadBytes(reinterpret_cast<void*>(&numOfSpacecrafts), 1);
	for (int craftIdx = 0; craftIdx < numOfSpacecrafts; ++craftIdx) {
		RoomManager::DeployData dd;
		ibs.ReadBytes(reinterpret_cast<void*>(&(dd.craft)), 1);
		ibs.ReadBytes(reinterpret_cast<void*>(&(dd.dir)), 1);
		ibs.ReadBytes(reinterpret_cast<void*>(&(dd.keyDeck.first)), 1);
		ibs.ReadBytes(reinterpret_cast<void*>(&(dd.keyDeck.second)), 1);
		deployData.push_back(dd);
		LOG_NOTIFY(
			"��ġ ����: �����ּ�(%s), ������ �ڵ�(%d), ����(%d), Ű ���� ��ǥ(%d, %d)",
			pCc->GetSocketAddr().ToString().c_str(),
			dd.craft,
			dd.dir,
			dd.keyDeck.first,
			dd.keyDeck.second
		);
	}

	mGameManager->InitializeDeploy(pCc, deployData);
	if (mGameManager->CheckDeployCompletion(pCc)) {
		// ��ġ ��� ����
		std::set<ClientCtxPtr> clients = mGameManager->GetParticipatingClients(pCc);
		//for (auto& c : clients) {
		//	std::vector<RoomManager::DeployData> ds = mGameManager->GetDeployStatus(c);
		//	OutputBitStream obs;
		//	PACKET_SIZE size = sizeof(PACKET_SIZE)
		//		+ sizeof(PACKET_TYPE)
		//		+ 1
		//		+ sizeof(RoomManager::DeployData) * ds.size();
		//	PACKET_TYPE type = SC_DEPLOY_RESULT;
		//	uint8_t numOfSpacecrafts = ds.size();
		//	obs.WriteBytes(reinterpret_cast<void*>(&size), 2);
		//	obs.WriteBytes(reinterpret_cast<void*>(&type), 1);
		//	obs.WriteBytes(reinterpret_cast<void*>(&numOfSpacecrafts), 1);
		//	for (auto& dd : ds) {
		//		obs.WriteBytes(reinterpret_cast<void*>(&(dd.craft)), 1);
		//		obs.WriteBytes(reinterpret_cast<void*>(&(dd.dir)), 1);
		//		obs.WriteBytes(reinterpret_cast<void*>(&(dd.keyDeck.first)), 1);
		//		obs.WriteBytes(reinterpret_cast<void*>(&(dd.keyDeck.second)), 1);
		//	}
		//	c->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
		//}

		// ��Ʋ ������ ���� �˸� ����
		std::string turnOwner = mGameManager->GetTurnOwner(pCc);
		for (auto& c : clients) {
			LOG_NOTIFY("��Ʋ������ ����: �����ּ�(%s)", c->GetSocketAddr().ToString().c_str());
			OutputBitStream obs;
			PACKET_SIZE size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1 + turnOwner.size();
			PACKET_TYPE type = SC_START_BP;
			uint8_t len = turnOwner.size();
			obs.WriteBytes(reinterpret_cast<void*>(&size), 2);
			obs.WriteBytes(reinterpret_cast<void*>(&type), 1);
			obs.WriteBytes(reinterpret_cast<void*>(&len), 1);
			obs.WriteBytes(reinterpret_cast<void*>(const_cast<char*>(turnOwner.c_str())), len);
			c->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
		}
	}
}


void NetworkManager::HandleRequestAttack(ClientCtxPtr pCc) {
	LOG_NOTIFY("���� ��û ����: �����ּ�(%s)", pCc->GetSocketAddr().ToString().c_str());

	const uint8_t* payload = pCc->GetPayload<PACKET_SIZE, PACKET_TYPE>();
	const uint32_t payloadSize = pCc->GetPayloadSizeInBits<PACKET_SIZE, PACKET_TYPE>();

	InputBitStream ibs(payload, payloadSize);

	uint8_t x, y;
	ibs.ReadBytes(reinterpret_cast<void*>(&x), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&y), 1);

	uint8_t resCode;
	switch (mGameManager->Attack(pCc, x, y)) {
	case 0:		// ����
		resCode = RES_FAILED;
		break;
	case 1:		// �̹� ħ���� ���� ����
		resCode = ALREADY_DESTROYED;
		break;
	case 2:		// ����
		resCode = RES_SUCCESS;
		break;
	}

	OutputBitStream obs;
	PACKET_SIZE size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1 + 1 + 1;
	PACKET_TYPE type = SC_RES_ATTACK;

	obs.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obs.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obs.WriteBytes(reinterpret_cast<void*>(&resCode), 1);
	obs.WriteBytes(reinterpret_cast<void*>(&x), 1);
	obs.WriteBytes(reinterpret_cast<void*>(&y), 1);

	pCc->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
}


void NetworkManager::HandleRequestSkill(ClientCtxPtr pCc) {
	LOG_NOTIFY("��ų ��û ����: �����ּ�(%s)", pCc->GetSocketAddr().ToString().c_str());

	const uint8_t* payload = pCc->GetPayload<PACKET_SIZE, PACKET_TYPE>();
	const uint32_t payloadSize = pCc->GetPayloadSizeInBits<PACKET_SIZE, PACKET_TYPE>();

	InputBitStream ibs(payload, payloadSize);

	uint8_t skill;
	ibs.ReadBytes(reinterpret_cast<void*>(&skill), 1);

	switch (skill) {
	case Skill::CANON:
	case Skill::ENHANCEMENT: {
		uint8_t x, y;
		ibs.ReadBytes(reinterpret_cast<void*>(&x), 1);
		ibs.ReadBytes(reinterpret_cast<void*>(&y), 1);
		mGameManager->CastSkill(pCc, skill, x, y);
		break;
	}
	case Skill::PORTAL: {
		mGameManager->CastSkill(pCc, skill);
		break;
	}
	case Skill::AMBUSH: {
		uint8_t x1, y1, x2, y2;
		ibs.ReadBytes(reinterpret_cast<void*>(&x1), 1);
		ibs.ReadBytes(reinterpret_cast<void*>(&y1), 1);
		ibs.ReadBytes(reinterpret_cast<void*>(&x2), 1);
		ibs.ReadBytes(reinterpret_cast<void*>(&y2), 1);
		mGameManager->CastSkill(pCc, skill, x1, y1, x2, y2);
		break;
	}
	}

	// TODO: �� ���� ���� ��, �� �ѱ� ��Ŷ ����
}