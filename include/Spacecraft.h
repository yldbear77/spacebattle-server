#ifndef __SPACE_CRAFT_H__
#define __SPACE_CRAFT_H__

#include <unordered_map>
#include <memory>
#include <utility>
#include <vector>
#include <string>

class Spacecraft {
public:
	enum {
		FIGHTER_JET = 1,
		DROPSHIP,
		STEALTH,
		MOTHER_SHIP,
		SPACE_STATION
	};

	enum {
		MSG_SUCCESS,		// 성공
		MSG_FAILED,			// 실패
		MSG_ARMORED,		// 강화된 곳
		MSG_DESTROYED		// 이미 침몰됨 (아머가 0인 상황)
	};

	Spacecraft(uint8_t code, std::string name) : mCode(code), mName(name)
	{
		for (int idx = 0; idx < craftInfo[code].second; ++idx) {
			mDecks.push_back(Deck{ 1, false });
		}
	}

	static std::unordered_map<uint8_t, std::pair<std::string, uint8_t>> craftInfo;
	static std::unordered_map<uint8_t, std::vector<std::vector<std::pair<int, int>>>> deployRule;

	const uint8_t mCode;
	const std::string mName;

	uint8_t GetDamaged(uint8_t deckNum) {
		switch (GetArmor(deckNum)) {
		case 0:
			return MSG_FAILED;
		default:
			DecreaseArmor(deckNum);
			if (GetEnhancedState(deckNum)) {
				mDecks[deckNum].isEnhanced = false;
				return MSG_ARMORED;
			}
			else return MSG_SUCCESS;
		}
	};

	uint8_t GetArmor(uint8_t deckNum) { return mDecks[deckNum].armor; }
	bool GetEnhancedState(uint8_t deckNum) { return mDecks[deckNum].isEnhanced; }
	void IncreaseArmor(uint8_t deckNum) { mDecks[deckNum].isEnhanced = true; ++(mDecks[deckNum].armor); }
	void DecreaseArmor(uint8_t deckNum) { --(mDecks[deckNum].armor); }

private:
	struct Deck {
		uint8_t armor;
		bool isEnhanced;
	};

	std::vector<Deck> mDecks;
};

typedef std::shared_ptr<Spacecraft> SpacecraftPtr;

#endif