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

class GameManager;

class WaitQueue {
public:
	WaitQueue() {}

	void Enqueue(ClientCtxPtr pCc, std::string name, uint8_t character, uint8_t mapCode);
	ClientCtxPtr Dequeue();

private:
	typedef struct {
		std::string name;
		uint8_t character;
		uint8_t mapCode;
	} WAITING_DATA;

	friend class GameManager;
	friend class RoomManager;

	std::queue<ClientCtxPtr> mQueue;
	std::mutex mMutex;

	std::unordered_map<ClientCtxPtr, WAITING_DATA> mWaitingData;
};

#endif