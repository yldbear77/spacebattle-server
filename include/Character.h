#ifndef __CHARACTER_H__
#define __CHARACTER_H__


#include <vector>
#include <memory>
#include <cstdarg>
#include <unordered_map>

#include "Spacecraft.h"
#include "ClientCtx.h"
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

	uint8_t GetCraftCount() const { return mCraftCount; }
	void CreateSpacecraft(uint8_t craft) {
		Spacecraft spc = Spacecraft(craft, Spacecraft::craftInfo[craft].first);
		mSpacecrafts.push_back(spc);
		++mCraftCount;
	}

	uint8_t BeAttacked(uint8_t craftNum, uint8_t deckNum) {
		return mSpacecrafts[craftNum].GetDamaged(deckNum);
	}

	virtual void dummy() = 0;

protected:
	uint8_t mCraftCount;
	std::vector<Spacecraft> mSpacecrafts;
};


// Kaiser decks = 
class Kaiser : public Character {
public:
	void dummy() override {};

	Canon::Result CastCanon(ClientCtxPtr pCc, uint8_t x, uint8_t y) { return mCanon.cast(pCc, x, y); }
	void CastEnhancement(uint8_t x, uint8_t y) {}

private:
	friend class RoomManager;

	Canon mCanon;
	Enhancement mEnhancement;

	Kaiser(uint8_t chCode = KAISER, std::string name = "Kaiser") :
		Character(chCode, name) {}
};


// Jack decks = 
class Jack : public Character {
public:
	void dummy() override {};

	void CastPortal() {}
	void CastAmbush() {}

private:
	friend class RoomManager;

	Portal mPortal;
	Ambush mAmbush;

	Jack(uint8_t chCode = JACK, std::string name = "Jack") :
		Character(chCode, name) {}
};


typedef std::shared_ptr<Character> CharacterPtr;


#endif