#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include <string>
#include <set>
#include <thread>

#include "Config.h"
#include "ClientCtx.h"
#include "WaitQueue.h"
#include "RoomManager.h"
#include "NetworkManager.h"
#include "Character.h"
#include "Skill.h"

class GameManager {
public:
	static GameManager* GetInstance() {
		if (mInstance == nullptr) mInstance = new GameManager();
		return mInstance;
	}

	void Run();

private:
	friend class NetworkManager;

	static GameManager* mInstance;
	GameManager();

	RoomManager* mRoomManager;
	NetworkManager* pNetworkManager;
	WaitQueue mQMapA, mQMapB;

	static void CreateGame(WaitQueue* pWaitQueue, GameManager* pGameManager);
	
	void EnqueueWaitingQ(ClientCtxPtr pCc, std::string name, uint8_t character, uint8_t mapCode);
	void InitializeDeploy(ClientCtxPtr pCc, std::vector<RoomManager::DeployData> deployData) {
		mRoomManager->InitializeDeploy(pCc, deployData);
	}
	bool CheckDeployCompletion(ClientCtxPtr pCc) { return mRoomManager->CheckDeployCompletion(pCc); }

	void ToggleTurnOwner(ClientCtxPtr pCc) { mRoomManager->ToggleTurnOwner(pCc); }

	std::vector<RoomManager::DeployData> GetDeployStatus(ClientCtxPtr pCc) { return mRoomManager->GetDeployStatus(pCc); }
	std::set<ClientCtxPtr> GetParticipatingClients(ClientCtxPtr pCc) { return mRoomManager->GetParticipatingClients(pCc); }
	uint16_t GetClientParticipatingRoom(ClientCtxPtr pCc) { return mRoomManager->GetClientParticipatingRoom(pCc); }
	std::string GetTurnOwner(ClientCtxPtr pCc) { return mRoomManager->GetTurnOwner(pCc); }
	std::string GetClientName(ClientCtxPtr pCc) { return mRoomManager->GetClientName(pCc); }
	ClientCtxPtr GetOpponent(ClientCtxPtr pCc) { return mRoomManager->GetOpponent(pCc); }
	uint8_t GetRemainingDecks(ClientCtxPtr pCc) { return mRoomManager->GetRemainingDecks(pCc); }

	uint8_t Attack(ClientCtxPtr pCc, uint8_t x, uint8_t y) { return mRoomManager->Attack(pCc, x, y); }

	Canon::Result CastCanon(ClientCtxPtr pCc, uint8_t x, uint8_t y) { return mRoomManager->CastCanon(pCc, x, y); }
	Enhancement::Result CastEnhancement(ClientCtxPtr pCc, uint8_t x, uint8_t y) { return mRoomManager->CastEnhancement(pCc, x, y); }
	Scan::Result CastScan(ClientCtxPtr pCc, uint8_t x, uint8_t y) { return mRoomManager->CastScan(pCc, x, y); }
	Ambush::Result CastAmbush(ClientCtxPtr pCc, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) { return mRoomManager->CastAmbush(pCc, x1, y1, x2, y2); }

	ClientCtxPtr GetWinner(ClientCtxPtr pCc) { return mRoomManager->GetWinner(pCc); }
};

#endif