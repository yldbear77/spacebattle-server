#include "IOCP.h"
#include "SocketUtil.h"

int32_t IOCP::ConnectSockToIOCP(TCPSocketPtr clntSock, SocketAddress& clntAddr) {
	LPIOCP_KEY_DATA pKeyData = new IOCP_KEY_DATA;
	pKeyData->pClntSock = clntSock;
	pKeyData->clntAddr = clntAddr;

	HANDLE hIOCP = CreateIoCompletionPort((HANDLE)clntSock->GetSocket(), mComPort, (ULONG_PTR)pKeyData, 0);
	if (hIOCP == NULL) {
		SocketUtil::ReportError("IOCP::ConnectSockToIOCP");
		return -SocketUtil::GetLastError();
	}

	return 0;
}

int32_t IOCP::GetCompletion(LPIOCP_KEY_DATA& pKeyData, LPEXT_OVERLAPPED& pIOData) {
	BOOL err;
	DWORD bytesTrans;

	err = GetQueuedCompletionStatus(mComPort, &bytesTrans, (PULONG_PTR)&pKeyData, (LPOVERLAPPED*)&pIOData, INFINITE);
	if (err == FALSE) {
		SocketUtil::ReportError("IOCP::GetCompletion");
		return -SocketUtil::GetLastError();
	}

	return bytesTrans;
}