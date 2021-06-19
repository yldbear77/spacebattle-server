#include "../include/Server.h"

Server* Server::mInstance = nullptr;

void Server::Run() {
	LOG_NOTIFY("SpaceBattle Server v0.1");
	LOG_NOTIFY("���� ����");
	mGameManager->Run();
	mNetworkManager->Run(mGameManager);
}