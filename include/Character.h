#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <vector>
#include <memory>
#include <unordered_map>

#include "Spacecraft.h"
#include "Skill.h"

class Character {
public:
	enum {
		JACK,
		KAISER
	};

	Character(uint8_t chCode, std::string name) :
		mCode(chCode), mName(name), mCraftCount(0) {}

	const uint8_t mCode;
	const std::string mName;

	uint8_t mCraftCount;
	std::vector<SpacecraftPtr> mCreatedCrafts;

	virtual void UseSkill() = 0;
};

class Jack : public Character {
public:
	void UseSkill() override {}

private:
	friend class RoomManager;

	Jack(uint8_t chCode = JACK, std::string name = "Jack") :
		Character(chCode, name) {}
};

class Kaiser : public Character {
public:
	void UseSkill() override {}

private:
	friend class RoomManager;

	Kaiser(uint8_t chCode = KAISER, std::string name = "Kaiser") :
		Character(chCode, name) {}
};

typedef std::shared_ptr<Character> CharacterPtr;

#endif