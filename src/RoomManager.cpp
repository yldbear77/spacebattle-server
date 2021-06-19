#include "../include/RoomManager.h"

RoomManager* RoomManager::mInstance = nullptr;

void RoomManager::Run() {
	LOG_NOTIFY("�� �Ŵ��� ����");
}

void RoomManager::CreateRoom(WaitQueue* pWaitQueue, ClientCtxPtr clientA, ClientCtxPtr clientB) {
	LOG_NOTIFY(
		"���� ����: �� ��ȣ(%d), ��(%d), Ŭ���̾�Ʈ(%s, %s), ĳ����(%d, %d)",
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