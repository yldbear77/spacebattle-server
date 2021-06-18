#include "../include/RoomManager.h"

RoomManager* RoomManager::mInstance = nullptr;

void RoomManager::CreateRoom(WaitQueue* pWaitQueue, ClientCtxPtr clientA, ClientCtxPtr clientB) {
	mRooms[++mRoomCount] = Room();
	
	Room& newRoom = mRooms[mRoomCount];
	newRoom.roomNumber = mRoomCount;
	newRoom.mapCode = pWaitQueue->mWaitingData[clientA].mapCode;
	newRoom.clients.insert(clientA);
	newRoom.clients.insert(clientB);
	newRoom.characters[clientA] = pWaitQueue->mWaitingData[clientA].character;
	newRoom.characters[clientB] = pWaitQueue->mWaitingData[clientB].character;
}