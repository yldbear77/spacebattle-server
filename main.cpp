#include "include/Server.h"
#include "net/SocketUtil.h"

int main(void) {
	if (SocketUtil::Init()) {
		Server::GetInstance()->Run();
		// TODO: ��� �Է� ���
		while (1);
	}
	return 0;
}