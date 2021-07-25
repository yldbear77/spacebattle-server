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

	std::vector<RoomManager::DeployData> GetDeployStatus(ClientCtxPtr pCc) { return mRoomManager->GetDeployStatus(pCc); }
	std::set<ClientCtxPtr> GetParticipatingClients(ClientCtxPtr pCc) { return mRoomManager->GetParticipatingClients(pCc); }
	std::string GetTurnOwner(ClientCtxPtr pCc) { return mRoomManager->GetTurnOwner(pCc); }

	uint8_t Attack(ClientCtxPtr pCc, uint8_t x, uint8_t y) { return mRoomManager->Attack(pCc, x, y); }
};

#endif