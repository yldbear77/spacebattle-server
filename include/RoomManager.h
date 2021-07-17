#ifndef __ROOM_MANAGER_H__
#define __ROOM_MANAGER_H__

#include <unordered_set>
#include <unordered_map>
#include <random>
#include <mutex>
#include <set>
#include <map>

#include "ClientCtx.h"
#include "WaitQueue.h"
#include "Character.h"
#include "Spacecraft.h"

#include "../util/Logger.h"

class RoomManager {
public:
	struct DeployData {
		uint8_t craft;
		uint8_t dir;
		std::pair<uint8_t, uint8_t> keyDeck;
	};

private:
	struct PosInfo {
		uint8_t craftNum;
		uint8_t deckNum;
	};

	struct Room {
		typedef std::map<std::pair<uint8_t, uint8_t>, PosInfo> GridMap;

		// 방에 대한 기본 정보
		uint16_t roomNumber;
		uint8_t	mapCode;
		std::set<ClientCtxPtr> clients;

		// 초기 배치 단계에 필요한 데이터
		uint8_t deployedNum;
		std::mutex deployCompletionCheckMtx;

		// 상대방 소켓주소, 클라이언트 별 캐릭터, 현재 배치 상태, target grid, ocean grid
		std::map<ClientCtxPtr, ClientCtxPtr> opponent;
		std::map<ClientCtxPtr, CharacterPtr> chs;
		std::map<ClientCtxPtr, std::vector<DeployData>> deloyStatus;
		std::map<ClientCtxPtr, GridMap> oceanGrid;
		std::map<ClientCtxPtr, GridMap> targetGrid;

		// 클라이언트 별 이름, 현재 턴 주인
		std::map<ClientCtxPtr, std::string> clientName;
		std::string turnOwner;
	};

public:
	static RoomManager* GetInstance() {
		if (mInstance == nullptr) mInstance = new RoomManager();
		return mInstance;
	}

	void Run();
	void CreateRoom(WaitQueue* pWaitQueue, ClientCtxPtr clientA, ClientCtxPtr clientB);

	CharacterPtr GetCharacterInfo(uint16_t roomNum, ClientCtxPtr pCc) { return mRooms[roomNum].chs[pCc]; }
	uint16_t GetClientParticipatingRoom(ClientCtxPtr pCc) { return mParticipatingRoom[pCc]; }
	std::set<ClientCtxPtr> GetParticipatingClients(ClientCtxPtr pCc) { return mRooms[GetClientParticipatingRoom(pCc)].clients; }

	void InitializeDeploy(ClientCtxPtr pCc, std::vector<DeployData>& data);
	bool CheckDeployCompletion(ClientCtxPtr pCc);

	std::vector<DeployData> GetDeployStatus(ClientCtxPtr pCc) { return mRooms[GetClientParticipatingRoom(pCc)].deloyStatus[pCc]; }
	std::string GetTurnOwner(ClientCtxPtr pCc) { return mRooms[GetClientParticipatingRoom(pCc)].turnOwner; }

	bool Attack(ClientCtxPtr pCc, uint8_t x, uint8_t y);

	//void Redeploy(ClientCtxPtr pCc, uint8_t newKeyDeckX, uint8_t newKeyDeckY) {}

private:
	typedef CharacterPtr (RoomManager::*CharacterCreatorPtr)();
	typedef std::unordered_map<uint8_t, CharacterCreatorPtr> CharacterCreatorPtrMap;

	static RoomManager* mInstance;
	RoomManager() : mRoomCount(0) {}

	std::random_device rd;
	std::mt19937 gen;

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