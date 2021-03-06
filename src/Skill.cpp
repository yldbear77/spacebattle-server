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

		if (opntOg.find(std::make_pair(nx, ny)) == opntOg.end()) continue;

		uint8_t res = rm->GetCharacterInfo(roomNum, opponent)->BeAttacked(
			opntOg[std::make_pair(nx, ny)].craftNum,
			opntOg[std::make_pair(nx, ny)].deckNum
		);

		if (res == Spacecraft::MSG_SUCCESS || res == Spacecraft::MSG_ARMORED) {
			skillResult.isSuccess = true;
			skillResult.coords.push_back(i);
		}
	}

	return skillResult;
}


std::pair<int, int> Scan::castRange[4] = {
	{0, 0}, {1, 0}, {0, 1}, {1, 1}
};


Scan::Result Scan::cast(ClientCtxPtr pCc, uint8_t x, uint8_t y) {
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

		if (opntOg.find(std::make_pair(nx, ny)) == opntOg.end()) continue;

		skillResult.isSuccess = true;
		skillResult.coords.push_back(i);
	}

	return skillResult;
}


Enhancement::Result Enhancement::cast(ClientCtxPtr pCc, uint8_t x, uint8_t y) {
	RoomManager* rm = RoomManager::GetInstance();

	uint16_t roomNum = rm->GetClientParticipatingRoom(pCc);
	auto& oceanGrid = rm->mRooms[roomNum].oceanGrid[pCc];

	Result skillResult;
	skillResult.isSuccess = false;

	if (oceanGrid.find(std::make_pair(x, y)) != oceanGrid.end()) {
		CharacterPtr ch = rm->GetCharacterInfo(roomNum, pCc);

		const uint8_t craftNum = oceanGrid[std::make_pair(x, y)].craftNum;
		const uint8_t deckNum = oceanGrid[std::make_pair(x, y)].deckNum;

		if (ch->GetDeckArmor(craftNum, deckNum) != 0) {
			ch->EnhanceDeckArmor(craftNum, deckNum);
			rm->IncreaseRemainingDeck(pCc);
			skillResult.isSuccess = true;
		}
	}

	return skillResult;
}


Ambush::Result Ambush::cast(ClientCtxPtr pCc, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
	RoomManager* rm = RoomManager::GetInstance();

	uint16_t roomNum = rm->GetClientParticipatingRoom(pCc);
	ClientCtxPtr opponent = rm->GetOpponent(pCc);
	auto& opntOg = rm->mRooms[roomNum].oceanGrid[opponent];

	Result skillResult;
	skillResult.isSuccess1 = skillResult.isSuccess2 = false;

	if (opntOg.find(std::make_pair(x1, y1)) != opntOg.end()) {
		uint8_t res = rm->GetCharacterInfo(roomNum, opponent)->BeAttacked(
			opntOg[std::make_pair(x1, y1)].craftNum,
			opntOg[std::make_pair(x1, y1)].deckNum
		);

		if (res == Spacecraft::MSG_SUCCESS || res == Spacecraft::MSG_ARMORED) {
			skillResult.isSuccess1 = true;
			skillResult.x1 = x1;
			skillResult.y1 = y1;
		}
	}

	if (opntOg.find(std::make_pair(x2, y2)) != opntOg.end()) {
		uint8_t res = rm->GetCharacterInfo(roomNum, opponent)->BeAttacked(
			opntOg[std::make_pair(x2, y2)].craftNum,
			opntOg[std::make_pair(x2, y2)].deckNum
		);

		if (res == Spacecraft::MSG_SUCCESS || res == Spacecraft::MSG_ARMORED) {
			skillResult.isSuccess2 = true;
			skillResult.x2 = x2;
			skillResult.y2 = y2;
		}
	}

	return skillResult;
}