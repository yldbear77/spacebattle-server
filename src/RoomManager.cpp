#include "../include/RoomManager.h"


RoomManager* RoomManager::mInstance = nullptr;


RoomManager::Room::Room(
	uint16_t roomNumber,
	uint8_t mapCode,
	ClientCtxPtr clientA,
	ClientCtxPtr clientB) :
	roomNumber(roomNumber),
	mapCode(mapCode)
{
	clients.insert(clientA);
	clients.insert(clientB);
}


void RoomManager::Run() {
	LOG_NOTIFY("룸 매니저 시작");
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
	mRooms[mRoomCount] = RoomManager::Room(
		mRoomCount,
		pWaitQueue->mWaitingData[clientA].mapCode,
		clientA,
		clientB);
	mRooms[mRoomCount].chs[clientA] =
		CreateCharacter(pWaitQueue->mWaitingData[clientA].character);
	mRooms[mRoomCount].chs[clientB] =
		CreateCharacter(pWaitQueue->mWaitingData[clientB].character);
	mParticipatingRoom[clientA] = mParticipatingRoom[clientB] = mRoomCount;
}


void RoomManager::InitializeDeploy(ClientCtxPtr pCc, std::vector<DeployData>& deployData) {
	uint16_t roomNum = GetClientParticipatingRoom(pCc);
	
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