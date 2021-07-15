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

	Spacecraft(uint8_t code, std::string name) : mCode(code), mName(name)
	{
		for (int idx = 0; idx < craftInfo[code].second; ++idx) {
			mDecks.push_back(Deck{ 1 });
		}
	}

	static std::unordered_map<uint8_t, std::pair<std::string, uint8_t>> craftInfo;
	static std::unordered_map<uint8_t, std::vector<std::vector<std::pair<int, int>>>> deployRule;

	const uint8_t mCode;
	const std::string mName;

	void BeAttacked() {};

private:
	struct Deck {
		uint8_t armor;
	};

	std::vector<Deck> mDecks;
};

typedef std::shared_ptr<Spacecraft> SpacecraftPtr;

#endif