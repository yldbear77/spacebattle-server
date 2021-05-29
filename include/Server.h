#ifndef __SERVER_H__
#define __SERVER_H__

#include "Config.h"
#include "NetworkManager.h"

class Server {
public:
	static Server* GetInstance() {
		if (mInstance == nullptr) mInstance = new Server();
		return mInstance;
	}

	void Run();

private:
	static Server* mInstance;

	NetworkManager* mNetworkManager;

	Server() { mNetworkManager = NetworkManager::GetInstance(); }
};

#endif