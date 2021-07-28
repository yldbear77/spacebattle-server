#include "../include/RoomManager.h"


RoomManager* RoomManager::mInstance = nullptr;


void RoomManager::Run() {
	LOG_NOTIFY("룸 매니저 시작");
	gen = std::mt19937(gen());
}


void RoomManager::CreateRoom(WaitQueue* pWaitQueue, ClientCtxPtr clientA, ClientCtxPtr clientB) {
	LOG_NOTIFY(
		"게임 생성: 방 번호(%d), 맵(%d), 클라이언트(%s, %s), 캐릭터(%d, %d)",
		mRoomCount + 1,
		pWaitQueue->mWaitingData[clientA].mapCode,
		pWaitQueue->mWaitingData[clientA].name.c_str(),
		pWaitQueue->mWaitingData[clientB].name.c_str(),
		pWaitQueue->mWaitingData[clientA].character,
		pWaitQueue->mWaitingData[clientB].character
	);
	++mRoomCount;

	if (mRooms.find(mRoomCount) == mRooms.end()) mRooms.erase(mRoomCount);

	mRooms[mRoomCount].roomNumber = mRoomCount;
	mRooms[mRoomCount].mapCode = pWaitQueue->mWaitingData[clientA].mapCode;
	mRooms[mRoomCount].deployedNum = 0;
	mRooms[mRoomCount].clients.insert(clientA);
	mRooms[mRoomCount].clients.insert(clientB);

	mRooms[mRoomCount].opponent[clientA] = clientB;
	mRooms[mRoomCount].opponent[clientB] = clientA;

	mRooms[mRoomCount].chs[clientA] =
		CreateCharacter(pWaitQueue->mWaitingData[clientA].character);
	mRooms[mRoomCount].chs[clientB] =
		CreateCharacter(pWaitQueue->mWaitingData[clientB].character);
	mParticipatingRoom[clientA] = mParticipatingRoom[clientB] = mRoomCount;

	const std::string clntAName = pWaitQueue->mWaitingData[clientA].name;
	const std::string clntBName = pWaitQueue->mWaitingData[clientB].name;

	mRooms[mRoomCount].clientName[clientA] = clntAName;
	mRooms[mRoomCount].clientName[clientB] = clntBName;

	std::uniform_int_distribution<int> dis(0, 1);
	(dis(gen) == 0) ? mRooms[mRoomCount].turnOwner = clntAName : mRooms[mRoomCount].turnOwner = clntBName;
}


void RoomManager::InitializeDeploy(ClientCtxPtr pCc, std::vector<DeployData>& deployData) {
	uint16_t roomNum = GetClientParticipatingRoom(pCc);

	CharacterPtr ch = GetCharacterInfo(roomNum, pCc);

	for (auto& dd : deployData) {
		uint8_t craftSize = Spacecraft::craftInfo[dd.craft].second;
		mRooms[roomNum].deloyStatus[pCc].push_back(dd);
		for (uint8_t deckIdx = 0; deckIdx < craftSize; ++deckIdx) {
			uint8_t x = dd.keyDeck.first + Spacecraft::deployRule[dd.craft][dd.dir][deckIdx].first;
			uint8_t y = dd.keyDeck.second + Spacecraft::deployRule[dd.craft][dd.dir][deckIdx].second;
			mRooms[roomNum].oceanGrid[pCc][std::make_pair(x, y)] = PosInfo{ ch->GetCraftCount(), deckIdx };
		}
		ch->CreateSpacecraft(dd.craft);
	}
}


bool RoomManager::CheckDeployCompletion(ClientCtxPtr pCc) {
	uint16_t roomNum = GetClientParticipatingRoom(pCc);
	mRooms[roomNum].deployCompletionCheckMtx.lock();
	if (++(mRooms[roomNum].deployedNum) >= 2) {
		mRooms[roomNum].deployedNum = 0;
		mRooms[roomNum].deployCompletionCheckMtx.unlock();
		return true;
	}
	mRooms[roomNum].deployCompletionCheckMtx.unlock();
	return false;
}


uint8_t RoomManager::Attack(ClientCtxPtr pCc, uint8_t x, uint8_t y) {
	uint16_t roomNum = GetClientParticipatingRoom(pCc);
	ClientCtxPtr opponentPCc = GetOpponent(pCc);

	if (mRooms[roomNum].oceanGrid[opponentPCc].find(std::make_pair(x, y)) ==
		mRooms[roomNum].oceanGrid[opponentPCc].end()) {
		return 0;
	}

	uint8_t res = mRooms[roomNum].chs[pCc]->BeAttacked(
		mRooms[roomNum].oceanGrid[opponentPCc][std::make_pair(x, y)].craftNum,
		mRooms[roomNum].oceanGrid[opponentPCc][std::make_pair(x, y)].deckNum
	);

	return res;
}


void RoomManager::CastCanon(ClientCtxPtr pCc, uint8_t x, uint8_t y) {

}


void RoomManager::CastPortal(ClientCtxPtr pCc) {

}


RoomManager::CharacterCreatorPtrMap* RoomManager::InitCharacterCreationPtrMap() {
	CharacterCreatorPtrMap* pCcpm = new CharacterCreatorPtrMap;

	(*pCcpm)[Character::JACK] = &RoomManager::CreateJack;
	(*pCcpm)[Character::KAISER] = &RoomManager::CreateKaiser;

	return pCcpm;
}


RoomManager::CharacterCreatorPtr RoomManager::CharacterCreatorLookup(uint8_t chCode) {
	static std::shared_ptr<CharacterCreatorPtrMap>
		creationPtrMap(InitCharacterCreationPtrMap());

	CharacterCreatorPtrMap::iterator mapEntry =
		creationPtrMap->find(chCode);

	if (mapEntry == creationPtrMap->end()) return nullptr;
	return (*mapEntry).second;
}


CharacterPtr RoomManager::CreateCharacter(uint8_t chCode) {
	CharacterCreatorPtr pCcp = CharacterCreatorLookup(chCode);

	if (pCcp) return (this->*pCcp)();
	else return nullptr;
}