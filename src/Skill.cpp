#include "../include/Skill.h"
#include "../include/RoomManager.h"


std::pair<int, int> Canon::castRange[4] = { 
	{0, 0}, {1, 0}, {0, 1}, {1, 1}
};


Canon::Result Canon::cast(ClientCtxPtr pCc, uint8_t x, uint8_t y) {
	RoomManager* rm = RoomManager::GetInstance();

	uint16_t roomNum = rm->GetClientParticipatingRoom(pCc);
	ClientCtxPtr opponent = rm->GetOpponent(pCc);
	auto& opntOg = rm->mRooms[roomNum].oceanGrid[opponent];

	Result skillResult;
	skillResult.isSuccess = false;
	skillResult.x = x;
	skillResult.y = y;

	for (int i = 0; i < 4; ++i) {
		uint8_t nx = x + castRange[i].first;
		uint8_t ny = y + castRange[i].second;

		if (opntOg.find(std::make_pair(x, y)) == opntOg.end()) continue;

		uint8_t res = rm->GetCharacterInfo(roomNum, opponent)->BeAttacked(
			opntOg[std::make_pair(x, y)].craftNum,
			opntOg[std::make_pair(x, y)].deckNum
		);
		if (res == 2) {
			skillResult.isSuccess = true;
			skillResult.coords.push_back(i);
		}
	}

	return skillResult;
}