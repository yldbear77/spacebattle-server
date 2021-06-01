#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include <queue>
#include <mutex>
#include <unordered_map>

#include "Config.h"
#include "ClientCtx.h"

class GameManager {
public:
	static GameManager* GetInstance() {
		if (mInstance == nullptr) mInstance = new GameManager();
		return mInstance;
	}

	void InsertWaitingQ(ClientCtxPtr pCc, std::string name, uint8_t character, uint8_t mapCode);

private:
	typedef struct {
		std::string name;
		uint8_t character;
	} WAITING_DATA;

	static GameManager* mInstance;
	GameManager() {};

	std::queue<ClientCtxPtr> mWaitingAQ, mWaitingBQ;
	std::mutex mLockA, mLockB;

	std::unordered_map<ClientCtxPtr, WAITING_DATA> mWaitingData;
};

#endif