#include "include/Server.h"
#include "net/SocketUtil.h"

int main(void) {
	if (SocketUtil::Init()) {
		Server::GetInstance()->Run();
		// TODO: 명령 입력 대기
	}
	return 0;
}