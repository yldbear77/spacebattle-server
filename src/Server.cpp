#include "../include/Server.h"

Server* Server::mInstance = nullptr;

void Server::Run() {
	LOG_NOTIFY("SpaceBattle Server v0.1");
	LOG_NOTIFY("서버 시작");
	mGameManager->Run();
	mNetworkManager->Run(mGameManager);
}