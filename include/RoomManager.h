#ifndef __ROOM_MANAGER_H__
#define __ROOM_MANAGER_H__

#include <unordered_set>
#include <unordered_map>

#include "ClientCtx.h"
#include "WaitQueue.h"
#include "Character.h"

#include "../util/Logger.h"

class RoomManager {
private:
	struct Room {
		typedef std::unordered_map<std::pair<uint8_t, uint8_t>, uint8_t> GridMap;
		Room() {}
		Room(uint16_t, uint8_t, ClientCtxPtr, ClientCtxPtr);
		uint16_t roomNumber;
		uint8_t	mapCode;
		std::unordered_set<ClientCtxPtr> clients;
		std::unordered_map<ClientCtxPtr, CharacterPtr> chs;
		std::unordered_map<ClientCtxPtr, GridMap> oceanGrid;
		std::unordered_map<ClientCtxPtr, GridMap> targetGrid;
	};

public:
	struct DeployData {
		uint8_t craft;
		uint8_t dir;
		std::pair<uint8_t, uint8_t> keyDeck;
	};

	static RoomManager* GetInstance() {
		if (mInstance == nullptr) mInstance = new RoomManager();
		return mInstance;
	}

	void Run();
	void CreateRoom(WaitQueue* pWaitQueue, ClientCtxPtr clientA, ClientCtxPtr clientB);

	CharacterPtr GetCharacterInfo(uint16_t roomNum, ClientCtxPtr pCc) { return mRooms[roomNum].chs[pCc]; }
	uint16_t GetClientParticipatingRoom(ClientCtxPtr pCc) { return mParticipatingRoom[pCc]; }

	void InitializeDeploy(ClientCtxPtr pCc, std::vector<DeployData>& data);

private:

	typedef CharacterPtr (RoomManager::*CharacterCreatorPtr)();
	typedef std::unordered_map<uint8_t, CharacterCreatorPtr> CharacterCreatorPtrMap;

	static RoomManager* mInstance;
	RoomManager() : mRoomCount(0) {}

	uint16_t mRoomCount;
	std::unordered_map<uint16_t, Room> mRooms;
	std::unordered_map<ClientCtxPtr, uint16_t> mParticipatingRoom;

	static CharacterCreatorPtrMap* InitCharacterCreationPtrMap();
	CharacterCreatorPtr CharacterCreatorLookup(uint8_t);
	CharacterPtr CreateCharacter(uint8_t);

	CharacterPtr CreateJack() { return std::shared_ptr<Character>(new Jack()); }
	CharacterPtr CreateKaiser() { return std::shared_ptr<Character>(new Kaiser()); }
};

#endif