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

	Spacecraft(uint8_t code, std::string name) : mCode(code), mName(name) {}

	static std::unordered_map<uint8_t, std::pair<std::string, uint8_t>> craftInfo;

	const uint8_t mCode;
	const std::string mName;

	virtual void Deploy() {};
	virtual void BeAttacked() {};
};

typedef std::shared_ptr<Spacecraft> SpacecraftPtr;

#endif