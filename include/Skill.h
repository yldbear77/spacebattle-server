#ifndef __SKILL_H__
#define __SKILL_H__

#include <cstdint>
#include <utility>
#include <vector>

enum Skill {
	CANON		= 0,
	PORTAL		= 1,
	ENHANCEMENT	= 2,
	AMBUSH		= 3,
};

class Canon {
public:
	struct Result {
		bool isSuccess;
		uint8_t x;
		uint8_t y;
		std::vector<uint8_t> coords;
	};

	void cast(uint8_t x, uint8_t y) {
		for (int i = 0; i < 4; ++i) {
			uint8_t nx = x + castRange[i].first;
			uint8_t ny = y + castRange[i].second;
		}
	}

private:
	static std::pair<int, int> castRange[4];
};

class Portal {
public:
	void cast() {}
};

class Enhancement {
public:
	void cast() {}
};

class Ambush {
public:
	void cast() {}
};

#endif