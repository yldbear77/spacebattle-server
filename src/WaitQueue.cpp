#include "../include/WaitQueue.h"

void WaitQueue::Enqueue(ClientCtxPtr pCc, std::string name, uint8_t character, uint8_t mapCode) {
	mMutex.lock();
	mQueue.push(pCc);
	mWaitingData[pCc] = WAITING_DATA{name, character};
	mMutex.unlock();
}

void WaitQueue::Dequeue(WaitQueue* pWaitQueue) {
	while (1) {
		pWaitQueue->mMutex.lock();
		if (pWaitQueue->mQueue.size() >= 2) {
			// TODO: 각 클라이언트의 상태 점검
			// TODO: 게임 방 생성 및 게임 시작 패킷 송신
		}
		pWaitQueue->mMutex.unlock();
	}
}