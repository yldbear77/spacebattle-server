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
			// TODO: �� Ŭ���̾�Ʈ�� ���� ����
			// TODO: ���� �� ���� �� ���� ���� ��Ŷ �۽�
		}
		pWaitQueue->mMutex.unlock();
	}
}