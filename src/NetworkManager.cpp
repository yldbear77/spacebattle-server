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
	LOG_NOTIFY("네트워크 매니저 시작");
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
			LOG_NOTIFY("클라이언트 종료: 소켓주소(%s)", pKeyData->clntAddr.ToString().c_str());
			// TODO: 게임 진행 중 종료하는 경우도 있으므로, 별도로 처리 필요
			pNetworkManager->DecreaseClntCount();
			pNetworkManager->mAddrToCcpMap[pKeyData->clntAddr]->UnsetOnlineStatus();
			pNetworkManager->mAddrToCcpMap.erase(pKeyData->clntAddr);
			continue;
		}
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

		LOG_NOTIFY("클라이언트 접속: 소켓주소(%s)", clntAddr.ToString().c_str());

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
	sizeA += 1;				// 패킷 타입
	sizeA += 1;				// B의 캐릭터
	sizeA += 1;				// B의 이름 길이 정보
	sizeA += nameB.size();	// B의 이름 길이
	uint8_t sizeNameB = nameB.size();

	OutputBitStream obsA;
	obsA.WriteBytes(reinterpret_cast<void*>(&sizeA), 2);
	obsA.WriteBytes(reinterpret_cast<void*>(&type), 1);
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
	obsB.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obsB.WriteBytes(reinterpret_cast<void*>(&chA), 1);
	obsB.WriteBytes(reinterpret_cast<void*>(&sizeNameA), 1);
	obsB.WriteBytes(reinterpret_cast<void*>(const_cast<char*>(nameA.c_str())), sizeNameA);

	LOG_NOTIFY("매칭 성공 전송: 소켓주소(%s)", clientA->GetSocketAddr().ToString().c_str());
	clientA->SendPacket(obsA.GetBufferPtr(), obsA.GetByteLength());

	LOG_NOTIFY("매칭 성공 전송: 소켓주소(%s)", clientB->GetSocketAddr().ToString().c_str());
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

	LOG_NOTIFY("배치 요청 전송: 소켓주소(%s)", clientA->GetSocketAddr().ToString().c_str());
	clientA->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());

	LOG_NOTIFY("배치 요청 전송: 소켓주소(%s)", clientB->GetSocketAddr().ToString().c_str());
	clientB->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
}


void NetworkManager::HandleRequestConnect(ClientCtxPtr pCc) {
	LOG_NOTIFY("큐 진입 요청 수신: 소켓주소(%s)", pCc->GetSocketAddr().ToString().c_str());

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
	uint8_t resCode = RES_SUCCESS;		// TODO: RES_FAILED가 될 수도 있는데, EnqueueWaiting과 연계된 문제

	obs.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obs.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obs.WriteBytes(reinterpret_cast<void*>(&resCode), 1);

	LOG_NOTIFY("큐 진입 응답 전송: 소켓주소(%s)", pCc->GetSocketAddr().ToString().c_str());
	pCc->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
}


void NetworkManager::HandleResponseDeploy(ClientCtxPtr pCc) {
	LOG_NOTIFY("배치 응답 수신: 소켓주소(%s)", pCc->GetSocketAddr().ToString().c_str());

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
			"배치 내용: 소켓주소(%s), 전투기 코드(%d), 방향(%d), 키 갑판 좌표(%d, %d)",
			pCc->GetSocketAddr().ToString().c_str(),
			dd.craft,
			dd.dir,
			dd.keyDeck.first,
			dd.keyDeck.second
		);
	}

	mGameManager->InitializeDeploy(pCc, deployData);
	if (mGameManager->CheckDeployCompletion(pCc)) {
		// 배치 결과 전송
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

		// 배틀 페이즈 시작 알림 전송
		std::string turnOwner = mGameManager->GetTurnOwner(pCc);
		for (auto& c : clients) {
			LOG_NOTIFY("배틀페이즈 시작: 소켓주소(%s)", c->GetSocketAddr().ToString().c_str());
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
	// TODO: 턴 주인이 맞는지 검증

	LOG_NOTIFY("공격 요청 수신: 소켓주소(%s)", pCc->GetSocketAddr().ToString().c_str());

	const uint8_t* payload = pCc->GetPayload<PACKET_SIZE, PACKET_TYPE>();
	const uint32_t payloadSize = pCc->GetPayloadSizeInBits<PACKET_SIZE, PACKET_TYPE>();

	InputBitStream ibs(payload, payloadSize);

	uint8_t x, y;
	ibs.ReadBytes(reinterpret_cast<void*>(&x), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&y), 1);

	uint8_t resCode = mGameManager->Attack(pCc, x, y);

	OutputBitStream obs;
	PACKET_SIZE size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1 + 1 + 1;
	PACKET_TYPE type = SC_RES_ATTACK;

	obs.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obs.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obs.WriteBytes(reinterpret_cast<void*>(&resCode), 1);
	obs.WriteBytes(reinterpret_cast<void*>(&x), 1);
	obs.WriteBytes(reinterpret_cast<void*>(&y), 1);

	std::set<ClientCtxPtr> clients = mGameManager->GetParticipatingClients(pCc);
	for (auto& c : clients) {
		LOG_NOTIFY("공격 결과 송신: 소켓주소(%s), %s, X(%d), Y(%d)",
			c->GetSocketAddr().ToString().c_str(),
			resCode == Spacecraft::MSG_FAILED ? "실패" : "성공",
			x,
			y);
		c->SendPacket(obs.GetBufferPtr(), obs.GetByteLength());
	}
}


void NetworkManager::HandleRequestSkill(ClientCtxPtr pCc) {
	// TODO: 턴 주인이 맞는지 검증

	LOG_NOTIFY("스킬 요청 수신: 소켓주소(%s)", pCc->GetSocketAddr().ToString().c_str());

	const uint8_t* payload = pCc->GetPayload<PACKET_SIZE, PACKET_TYPE>();
	const uint32_t payloadSize = pCc->GetPayloadSizeInBits<PACKET_SIZE, PACKET_TYPE>();

	InputBitStream ibs(payload, payloadSize);

	uint8_t skill;
	ibs.ReadBytes(reinterpret_cast<void*>(&skill), 1);

	OutputBitStream obsToCaster, obsToCastee;

	switch (skill) {
	case Skill::CANON:
		HandleCanonSkill(pCc, ibs, obsToCaster, obsToCastee);
		break;
	case Skill::ENHANCEMENT:
		HandleEnhancementSkill(pCc, ibs, obsToCaster, obsToCastee);
		break;
	case Skill::SCAN:
		HandleScanSkill(pCc, ibs, obsToCaster, obsToCastee);
		break;
	case Skill::AMBUSH:
		HandleAmbushSkill(pCc, ibs, obsToCaster, obsToCastee);
		break;
	case Skill::NONE:
		break;
	}

	std::set<ClientCtxPtr> clients = mGameManager->GetParticipatingClients(pCc);
	for (auto& c : clients) {
		if(c == pCc) c->SendPacket(obsToCaster.GetBufferPtr(), obsToCaster.GetByteLength());
		else c->SendPacket(obsToCastee.GetBufferPtr(), obsToCastee.GetByteLength());
	}

	ClientCtxPtr opntPCc = mGameManager->GetOpponent(pCc);

	LOG_NOTIFY("%s의 남아있는 갑판 수: %d", mGameManager->GetClientName(pCc).c_str(), mGameManager->GetRemainingDecks(pCc));
	LOG_NOTIFY("%s의 남아있는 갑판 수: %d", mGameManager->GetClientName(opntPCc).c_str(), mGameManager->GetRemainingDecks(opntPCc));

	ClientCtxPtr winner = mGameManager->GetWinner(pCc);
	if (winner != nullptr) {
		LOG_NOTIFY("%d번 방 게임 종료: 승자(%s)",
			mGameManager->GetClientParticipatingRoom(winner),
			mGameManager->GetClientName(winner).c_str());

		for (auto& c : clients) {
			if (c == winner) {
				OutputBitStream winnerObs;

				PACKET_SIZE size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1;
				PACKET_TYPE type = SC_GAME_RESULT;
				uint8_t resCode = WINNER;

				winnerObs.WriteBytes(reinterpret_cast<void*>(&size), 2);
				winnerObs.WriteBytes(reinterpret_cast<void*>(&type), 1);
				winnerObs.WriteBytes(reinterpret_cast<void*>(&resCode), 1);

				c->SendPacket(winnerObs.GetBufferPtr(), winnerObs.GetByteLength());
			}
			else {
				OutputBitStream loserObs;

				PACKET_SIZE size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1;
				PACKET_TYPE type = SC_GAME_RESULT;
				uint8_t resCode = LOSER;

				loserObs.WriteBytes(reinterpret_cast<void*>(&size), 2);
				loserObs.WriteBytes(reinterpret_cast<void*>(&type), 1);
				loserObs.WriteBytes(reinterpret_cast<void*>(&resCode), 1);

				c->SendPacket(loserObs.GetBufferPtr(), loserObs.GetByteLength());
			}
		}
	}
	else {
		LOG_NOTIFY("%d번 방 턴 넘어감: 이름(%s -> %s)",
			mGameManager->GetClientParticipatingRoom(pCc),
			mGameManager->GetClientName(pCc).c_str(),
			mGameManager->GetClientName(opntPCc).c_str()
		);

		mGameManager->ToggleTurnOwner(pCc);
		std::string turnOwner = mGameManager->GetTurnOwner(pCc);

		OutputBitStream toggleTurnObs;
		PACKET_SIZE size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1 + turnOwner.size();
		PACKET_TYPE type = SC_START_BP;
		uint8_t len = turnOwner.size();

		toggleTurnObs.WriteBytes(reinterpret_cast<void*>(&size), 2);
		toggleTurnObs.WriteBytes(reinterpret_cast<void*>(&type), 1);
		toggleTurnObs.WriteBytes(reinterpret_cast<void*>(&len), 1);
		toggleTurnObs.WriteBytes(reinterpret_cast<void*>(const_cast<char*>(turnOwner.c_str())), len);

		for (auto& c : clients) c->SendPacket(toggleTurnObs.GetBufferPtr(), toggleTurnObs.GetByteLength());
	}
}


void NetworkManager::HandleCanonSkill(ClientCtxPtr pCc, InputBitStream& ibs, OutputBitStream& obsToCaster, OutputBitStream& obsToCastee) {
	uint8_t x, y;
	ibs.ReadBytes(reinterpret_cast<void*>(&x), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&y), 1);

	Canon::Result res = mGameManager->CastCanon(pCc, x, y);

	if (res.isSuccess) {
		LOG_NOTIFY("캐논 스킬 결과 송신: 소켓주소(%s), 성공, X(%d), Y(%d), 개수(%d)",
			pCc->GetSocketAddr().ToString().c_str(),
			x,
			y,
			res.coords.size()
		);
	}
	else {
		LOG_NOTIFY("캐논 스킬 결과 송신: 소켓주소(%s), 실패, X(%d), Y(%d)",
			pCc->GetSocketAddr().ToString().c_str(),
			res.isSuccess,
			x,
			y
		);
	}

	if (res.isSuccess) {
		PACKET_SIZE size = sizeof(PACKET_SIZE)
			+ sizeof(PACKET_TYPE)
			+ 1		// skill type
			+ 1		// res
			+ 1		// x
			+ 1		// y
			+ 1		// count
			+ res.coords.size();
		PACKET_TYPE type = SC_RES_SKILL_CASTER;
		uint8_t skillType = Skill::CANON;
		uint8_t resCode = !(res.isSuccess);

		obsToCaster.WriteBytes(reinterpret_cast<void*>(&size), 2);
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&type), 1);
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&skillType), 1);
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&(resCode)), 1);
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&x), 1);
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&y), 1);

		type = SC_RES_SKILL_CASTEE;

		obsToCastee.WriteBytes(reinterpret_cast<void*>(&size), 2);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&type), 1);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&skillType), 1);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&(resCode)), 1);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&x), 1);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&y), 1);

		uint8_t count = res.coords.size();
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&count), 1);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&count), 1);
		for (auto& c : res.coords) {
			obsToCaster.WriteBytes(reinterpret_cast<void*>(&(c)), 1);
			obsToCastee.WriteBytes(reinterpret_cast<void*>(&(c)), 1);
		}
	}
	else {
		PACKET_SIZE size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1 + 1 + 2;
		PACKET_TYPE type = SC_RES_SKILL_CASTER;
		uint8_t skillType = Skill::CANON;
		uint8_t resCode = !(res.isSuccess);

		obsToCaster.WriteBytes(reinterpret_cast<void*>(&size), 2);
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&type), 1);
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&skillType), 1);
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&(resCode)), 1);
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&x), 1);
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&y), 1);

		type = SC_RES_SKILL_CASTEE;

		obsToCastee.WriteBytes(reinterpret_cast<void*>(&size), 2);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&type), 1);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&skillType), 1);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&(resCode)), 1);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&x), 1);
		obsToCastee.WriteBytes(reinterpret_cast<void*>(&y), 1);
	}
}


void NetworkManager::HandleEnhancementSkill(ClientCtxPtr pCc, InputBitStream& ibs, OutputBitStream& obsToCaster, OutputBitStream& obsToCastee) {
	uint8_t x, y;
	ibs.ReadBytes(reinterpret_cast<void*>(&x), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&y), 1);

	Enhancement::Result res = mGameManager->CastEnhancement(pCc, x, y);

	LOG_NOTIFY("강화 스킬 결과 송신: 소켓주소(%s), %s, X(%d), Y(%d)",
		pCc->GetSocketAddr().ToString().c_str(),
		res.isSuccess ? "성공" : "실패",
		x, y
	);

	PACKET_SIZE size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1 + 1;
	PACKET_TYPE type = SC_RES_SKILL_CASTER;
	uint8_t skillType = Skill::ENHANCEMENT;
	uint8_t resCode = !res.isSuccess;

	obsToCaster.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&skillType), 1);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&resCode), 1);

	size -= 1;
	type = SC_RES_SKILL_CASTEE;

	obsToCastee.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&skillType), 1);
}


void NetworkManager::HandleScanSkill(ClientCtxPtr pCc, InputBitStream& ibs, OutputBitStream& obsToCaster, OutputBitStream& obsToCastee) {
	uint8_t x, y;
	ibs.ReadBytes(reinterpret_cast<void*>(&x), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&y), 1);

	Scan::Result res = mGameManager->CastScan(pCc, x, y);

	if (res.isSuccess) {
		LOG_NOTIFY("스캔 스킬 결과 송신: 소켓주소(%s), 성공, 개수(%d)",
			pCc->GetSocketAddr().ToString().c_str(),
			res.coords.size()
		);
	}
	else {
		LOG_NOTIFY("스캔 스킬 결과 송신: 소켓주소(%s), 실패",
			pCc->GetSocketAddr().ToString().c_str()
		);
	}

	PACKET_SIZE size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1;
	PACKET_TYPE type = SC_RES_SKILL_CASTER;
	uint8_t skillType = Skill::SCAN;

	size += 1 + 1 + 1 + res.coords.size();
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&skillType), 1);

	obsToCaster.WriteBytes(reinterpret_cast<void*>(&x), 1);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&y), 1);

	uint8_t count = res.coords.size();
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&count), 1);
	for (auto& c : res.coords) {
		obsToCaster.WriteBytes(reinterpret_cast<void*>(&(c)), 1);
	}

	size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1;
	type = SC_RES_SKILL_CASTEE;

	obsToCastee.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&skillType), 1);
}


void NetworkManager::HandleAmbushSkill(ClientCtxPtr pCc, InputBitStream& ibs, OutputBitStream& obsToCaster, OutputBitStream& obsToCastee) {
	uint8_t x1, y1, x2, y2;
	ibs.ReadBytes(reinterpret_cast<void*>(&x1), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&y1), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&x2), 1);
	ibs.ReadBytes(reinterpret_cast<void*>(&y2), 1);

	Ambush::Result res = mGameManager->CastAmbush(pCc, x1, y1, x2, y2);

	LOG_NOTIFY("기습 스킬 결과 송신: 소켓주소(%s), 성공여부1(%s), 성공여부2(%s)",
		pCc->GetSocketAddr().ToString().c_str(),
		res.isSuccess1 ? "성공" : "실패",
		res.isSuccess2 ? "성공" : "실패"
	);

	PACKET_SIZE size = sizeof(PACKET_SIZE) + sizeof(PACKET_TYPE) + 1;
	PACKET_TYPE type = SC_RES_SKILL_CASTER;
	uint8_t skillType = Skill::AMBUSH;

	size += 2 + 2 + 2;

	obsToCaster.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&skillType), 1);

	type = SC_RES_SKILL_CASTEE;

	obsToCastee.WriteBytes(reinterpret_cast<void*>(&size), 2);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&type), 1);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&skillType), 1);

	uint8_t resCode = !(res.isSuccess1);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&resCode), 1);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&resCode), 1);

	obsToCaster.WriteBytes(reinterpret_cast<void*>(&x1), 1);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&y1), 1);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&x1), 1);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&y1), 1);

	resCode = !(res.isSuccess2);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&resCode), 1);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&resCode), 1);

	obsToCaster.WriteBytes(reinterpret_cast<void*>(&x2), 1);
	obsToCaster.WriteBytes(reinterpret_cast<void*>(&y2), 1);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&x2), 1);
	obsToCastee.WriteBytes(reinterpret_cast<void*>(&y2), 1);
}