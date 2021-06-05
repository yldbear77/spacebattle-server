#include "../include/GameManager.h"

GameManager* GameManager::mInstance = nullptr;

void GameManager::EnqueueWaitingQ(ClientCtxPtr pCc, std::string name, uint8_t character, uint8_t mapCode) {
	switch (mapCode) {
	case MAP_10_10:
		mQMapA.Enqueue(pCc, name, character, mapCode);
		break;
	case MAP_20_20:
		mQMapB.Enqueue(pCc, name, character, mapCode);
		break;
	}
}