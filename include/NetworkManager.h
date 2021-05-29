#ifndef __NETWORK_MANAGER_H__
#define __NETWORK_MANAGER_H__

#include <unordered_map>

#include <thread>

#include "Config.h"
#include "Packet.h"
#include "ClientCtx.h"

#include "../net/SocketAddress.h"
#include "../net/TCPSocket.h"
#include "../net/IOCP.h"
#include "../net/SocketUtil.h"

class NetworkManager {
public:
	static NetworkManager* GetInstance() {
		if (mInstance == nullptr) {
			mInstance = new NetworkManager();
		}
		return mInstance;
	}

	void Run();

	void RegisterClient(TCPSocketPtr pClntSock, SocketAddress clntAddr) {
		ClientCtxPtr newClient = std::make_shared<ClientCtx>(pClntSock, clntAddr);
		mAddrToCcpMap[clntAddr] = newClient;
	}

	void ConstructPacket(SocketAddress clntAddr, uint8_t* ioBuffer, int remainingBytes);
	void ProcessPacket(SocketAddress clntAddr);

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

	static void PacketWorker(IOCPPtr pIOCP, NetworkManager* pNm);
	static void AcceptWorker(TCPSocketPtr pSock, IOCPPtr pIOCP, NetworkManager* pNm);
};

#endif