#include "../include/Server.h"

Server* Server::mInstance = nullptr;

void Server::Run() {
	mGameManager->Run();
	mNetworkManager->Run(mGameManager);
}