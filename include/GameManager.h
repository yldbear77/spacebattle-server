#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include <string>
#include <thread>

#include "Config.h"
#include "ClientCtx.h"
#include "WaitQueue.h"
#include "RoomManager.h"
#include "NetworkManager.h"

class GameManager {
public:
	static GameManager* GetInstance() {
		if (mInstance == nullptr) mInstance = new GameManager();
		return mInstance;
	}

	void Run();
	void EnqueueWaitingQ(ClientCtxPtr pCc, std::string name, uint8_t character, uint8_t mapCode);

private:
	static GameManager* mInstance;
	GameManager() {};

	RoomManager* mRoomManager;
	NetworkManager* pNetworkManager;
	WaitQueue mQMapA, mQMapB;

	static void CreateGame(WaitQueue* pWaitQueue, GameManager* pGameManager);
};

#endif