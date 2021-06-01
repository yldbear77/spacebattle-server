#include "../include/GameManager.h"

GameManager* GameManager::mInstance = nullptr;

void GameManager::InsertWaitingQ(ClientCtxPtr pCc, std::string name, uint8_t character, uint8_t mapCode) {
	WAITING_DATA waitingData = { name, character };
	switch (mapCode) {
	case MAP_10_10:
		mLockA.lock();
		mWaitingAQ.push(pCc);
		mLockA.unlock();
		break;
	case MAP_20_20:
		mLockB.lock();
		mWaitingBQ.push(pCc);
		mLockB.unlock();
		break;
	}
	mWaitingData[pCc] = waitingData;
}