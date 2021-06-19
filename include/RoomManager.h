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
		uint16_t roomNumber;									// �� ��ȣ
		uint8_t	mapCode;										// ��
		std::unordered_set<ClientCtxPtr> clients;				// �濡 ���Ե� Ŭ���̾�Ʈ���� ����
		std::unordered_map<ClientCtxPtr, uint8_t> characters;	// Ŭ���̾�Ʈ�� ���� ĳ����
		// TODO: �÷��̾ �������� ���� ����
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