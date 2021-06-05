#ifndef __WAIT_QUEUE_H__
#define __WAIT_QUEUE_H__

#include <queue>
#include <mutex>
#include <thread>
#include <string>
#include <unordered_map>
#include <map>

#include "Config.h"
#include "ClientCtx.h"

class WaitQueue {
public:
	WaitQueue() { std::thread(Dequeue, this).detach(); }

	void Enqueue(ClientCtxPtr pCc, std::string name, uint8_t character, uint8_t mapCode);

private:
	typedef struct {
		std::string name;
		uint8_t character;
	} WAITING_DATA;

	std::queue<ClientCtxPtr> mQueue;
	std::mutex mMutex;

	std::map<ClientCtxPtr, WAITING_DATA> mWaitingData;

	static void Dequeue(WaitQueue* pWaitQueue);
};

#endif