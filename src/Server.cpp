#include "../include/Server.h"

Server* Server::mInstance = nullptr;

void Server::Run() {
	mNetworkManager->Run(mGameManager);
}