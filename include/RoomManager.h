#ifndef __ROOM_MANAGER_H__
#define __ROOM_MANAGER_H__

#include <unordered_set>
#include <unordered_map>

#include "ClientCtx.h"
#include "WaitQueue.h"

#include "../util/Logger.h"

class RoomManager {
private:
	struct Room {
		uint16_t roomNumber;									// 방 번호
		uint8_t	mapCode;										// 맵
		std::unordered_set<ClientCtxPtr> clients;				// 방에 포함된 클라이언트들의 정보
		std::unordered_map<ClientCtxPtr, uint8_t> characters;	// 클라이언트별 선택 캐릭터
		// TODO: 플레이어별 종속적인 정보 저장
	};

public:
	static RoomManager* GetInstance() {
		if (mInstance == nullptr) mInstance = new RoomManager();
		return mInstance;
	}

	void Run();
	void CreateRoom(WaitQueue* pWaitQueue, ClientCtxPtr clientA, ClientCtxPtr clientB);

private:
	static RoomManager* mInstance;
	RoomManager() : mRoomCount(0) {}

	uint16_t mRoomCount;
	std::unordered_map<uint16_t, Room> mRooms;
};

#endif