#include "../include/WaitQueue.h"

void WaitQueue::Enqueue(ClientCtxPtr pCc, std::string name, uint8_t character, uint8_t mapCode) {
	mMutex.lock();
	mQueue.push(pCc);
	mWaitingData[pCc] = WAITING_DATA{name, character, mapCode};
	mMutex.unlock();
}

ClientCtxPtr WaitQueue::Dequeue() { 
	ClientCtxPtr tempCcp = mQueue.front();
	mQueue.pop();
	return tempCcp;
}