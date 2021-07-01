#include "../include/GameManager.h"


GameManager* GameManager::mInstance = nullptr;


GameManager::GameManager() {
	mRoomManager = RoomManager::GetInstance();
	pNetworkManager = NetworkManager::GetInstance();
}


void GameManager::Run() {
	LOG_NOTIFY("게임 매니저 시작");
	mRoomManager->Run();
	LOG_NOTIFY("게임 생성 스레드 시작");
	std::thread(CreateGame, &mQMapA, this).detach();
	std::thread(CreateGame, &mQMapB, this).detach();
}


void GameManager::CreateGame(WaitQueue* pWaitQueue, GameManager* pGameManager) {
	while (1) {
		pWaitQueue->mMutex.lock();
		if (pWaitQueue->mQueue.size() >= 2) {
			ClientCtxPtr clientA, clientB;

			clientA = pWaitQueue->Dequeue();
			clientB = pWaitQueue->Dequeue();

			if (!clientA->GetOnlineStatus() || !clientB->GetOnlineStatus()) {
				if (!clientA->GetOnlineStatus() && !clientB->GetOnlineStatus()) {
					pWaitQueue->mWaitingData.erase(clientA);
					pWaitQueue->mWaitingData.erase(clientB);
				}
				else if (!clientA->GetOnlineStatus()) {
					pWaitQueue->mWaitingData.erase(clientA);
					pWaitQueue->Enqueue(
						clientB,
						pWaitQueue->mWaitingData[clientB].name,
						pWaitQueue->mWaitingData[clientB].character,
						pWaitQueue->mWaitingData[clientB].mapCode
					);
				}
				else if (!clientB->GetOnlineStatus()) {
					pWaitQueue->mWaitingData.erase(clientB);
					pWaitQueue->Enqueue(
						clientA,
						pWaitQueue->mWaitingData[clientA].name,
						pWaitQueue->mWaitingData[clientA].character,
						pWaitQueue->mWaitingData[clientA].mapCode
					);
				}
			}
			else {
				pGameManager->mRoomManager->CreateRoom(pWaitQueue, clientA, clientB);

				uint8_t chA = pWaitQueue->mWaitingData[clientA].character;
				std::string nameA = pWaitQueue->mWaitingData[clientA].name;
				uint8_t chB = pWaitQueue->mWaitingData[clientB].character;
				std::string nameB = pWaitQueue->mWaitingData[clientB].name;

				pGameManager->pNetworkManager->SendMatchSuccessPacket(clientA, clientB, chA, nameA, chB, nameB);
				pGameManager->pNetworkManager->SendRequestDeployPacket(clientA, clientB);
			}
		}
		pWaitQueue->mMutex.unlock();
	}
}


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