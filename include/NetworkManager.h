#ifndef __NETWORK_MANAGER_H__
#define __NETWORK_MANAGER_H__

#include <unordered_map>
#include <utility>
#include <string>
#include <thread>

#include "Config.h"
#include "Packet.h"
#include "ClientCtx.h"
#include "GameManager.h"

#include "../include/Spacecraft.h"

#include "../net/SocketAddress.h"
#include "../net/TCPSocket.h"
#include "../net/IOCP.h"
#include "../net/SocketUtil.h"
#include "../net/BitStream.h"

#include "../util/Logger.h"

class NetworkManager {
public:
	static NetworkManager* GetInstance() {
		if (mInstance == nullptr) mInstance = new NetworkManager();
		return mInstance;
	}

	void Run(GameManager* pGameManager);

	void RegisterClient(TCPSocketPtr pClntSock, SocketAddress clntAddr) {
		ClientCtxPtr newClient = std::make_shared<ClientCtx>(pClntSock, clntAddr);
		mAddrToCcpMap[clntAddr] = newClient;
	}

	void ConstructPacket(SocketAddress clntAddr, uint8_t* ioBuffer, int remainingBytes);
	void ProcessPacket(ClientCtxPtr pCc);

	void SendMatchSuccessPacket(ClientCtxPtr, ClientCtxPtr, uint8_t, std::string, uint8_t, std::string);
	void SendRequestDeployPacket(ClientCtxPtr clientA, ClientCtxPtr clientB);

	void DecreaseClntCount() { --mClntCounts; }
	void IncreaseClntCount() { ++mClntCounts; }

private:
	static NetworkManager* mInstance;
	NetworkManager();

	const int mMaxClnts;
	const unsigned long mServIP;
	const short mServPort;

	int mClntCounts;

	SYSTEM_INFO mSysInfo;

	TCPSocketPtr mSock;
	IOCPPtr mIOCP;
	SocketAddress mAddr;

	std::unordered_map<SocketAddress, ClientCtxPtr> mAddrToCcpMap;

	GameManager* mGameManager;

	static void PacketWorker(IOCPPtr pIOCP, NetworkManager* pNetworkManager);
	static void AcceptWorker(TCPSocketPtr pSock, IOCPPtr pIOCP, NetworkManager* pNetworkManager);

	void HandleRequestConnect(ClientCtxPtr pCc);
	void HandleResponseDeploy(ClientCtxPtr pCc);
};

#endif