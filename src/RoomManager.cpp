#include "../include/RoomManager.h"

RoomManager* RoomManager::mInstance = nullptr;

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
	mRooms[++mRoomCount] = Room();
	
	Room& newRoom = mRooms[mRoomCount];
	newRoom.roomNumber = mRoomCount;
	newRoom.mapCode = pWaitQueue->mWaitingData[clientA].mapCode;
	newRoom.clients.insert(clientA);
	newRoom.clients.insert(clientB);
	newRoom.characters[clientA] = pWaitQueue->mWaitingData[clientA].character;
	newRoom.characters[clientB] = pWaitQueue->mWaitingData[clientB].character;
}