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
#include "Skill.h"

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

public:
	typedef std::map<std::pair<uint8_t, uint8_t>, PosInfo> GridMap;

private:
	struct Room {
		// �濡 ���� �⺻ ����
		uint16_t roomNumber;
		uint8_t	mapCode;
		std::set<ClientCtxPtr> clients;

		// �ʱ� ��ġ �ܰ迡 �ʿ��� ������
		uint8_t deployedNum;
		std::mutex deployCompletionCheckMtx;

		// ���� �����ּ�, Ŭ���̾�Ʈ �� ĳ����, ���� ��ġ ����, target grid, ocean grid
		std::map<ClientCtxPtr, ClientCtxPtr> opponent;
		std::map<ClientCtxPtr, CharacterPtr> chs;
		std::map<ClientCtxPtr, std::vector<DeployData>> deloyStatus;
		std::map<ClientCtxPtr, GridMap> oceanGrid;
		std::map<ClientCtxPtr, GridMap> targetGrid;

		// Ŭ���̾�Ʈ �� �̸�, ���� �� ����
		std::map<ClientCtxPtr, std::string> clientName;
		std::string turnOwner;

		// Ŭ���̾�Ʈ �� �����ִ� ���� ��
		std::map<ClientCtxPtr, uint8_t> remainingDecks;

		// ����
		ClientCtxPtr winnerPCc;
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
	std::string GetClientName(ClientCtxPtr pCc) { return mRooms[GetClientParticipatingRoom(pCc)].clientName[pCc]; }

	void InitializeDeploy(ClientCtxPtr pCc, std::vector<DeployData>& data);
	bool CheckDeployCompletion(ClientCtxPtr pCc);

	void ToggleTurnOwner(ClientCtxPtr pCc);

	void IncreaseRemainingDeck(ClientCtxPtr pCc) { ++(mRooms[GetClientParticipatingRoom(pCc)].remainingDecks[pCc]); }

	std::vector<DeployData> GetDeployStatus(ClientCtxPtr pCc) { return mRooms[GetClientParticipatingRoom(pCc)].deloyStatus[pCc]; }
	std::string GetTurnOwner(ClientCtxPtr pCc) { return mRooms[GetClientParticipatingRoom(pCc)].turnOwner; }
	ClientCtxPtr GetOpponent(ClientCtxPtr pCc) {
		// TODO: Ŭ���̾�Ʈ�� ���ӿ� ���� ������ ��ȿ�� ���� �ʿ�
		return mRooms[GetClientParticipatingRoom(pCc)].opponent[pCc];
	}
	uint8_t GetRemainingDecks(ClientCtxPtr pCc) { return mRooms[GetClientParticipatingRoom(pCc)].remainingDecks[pCc]; }

	ClientCtxPtr GetWinner(ClientCtxPtr pCc) { return mRooms[GetClientParticipatingRoom(pCc)].winnerPCc; }

	uint8_t Attack(ClientCtxPtr pCc, uint8_t x, uint8_t y);

	Canon::Result CastCanon(ClientCtxPtr pCc, uint8_t x, uint8_t y);
	Enhancement::Result CastEnhancement(ClientCtxPtr pCc, uint8_t x, uint8_t y);
	Scan::Result CastScan(ClientCtxPtr pCc, uint8_t x, uint8_t y);
	Ambush::Result CastAmbush(ClientCtxPtr pCc, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

private:
	typedef CharacterPtr (RoomManager::*CharacterCreatorPtr)();
	typedef std::unordered_map<uint8_t, CharacterCreatorPtr> CharacterCreatorPtrMap;

	friend class Canon;
	friend class Scan;
	friend class Enhancement;
	friend class Ambush;

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

	void DecreaseRemainingDecks(ClientCtxPtr pCc, uint8_t count);
};

#endif