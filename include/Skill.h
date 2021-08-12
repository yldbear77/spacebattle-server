#ifndef __SKILL_H__
#define __SKILL_H__


#include <cstdint>
#include <utility>
#include <vector>

#include "ClientCtx.h"


enum Skill {
	CANON		= 0,
	PORTAL		= 1,
	ENHANCEMENT	= 2,
	AMBUSH		= 3,
};


class Canon {
public:
	struct Data {
		uint8_t x;
		uint8_t y;
	};

	struct Result {
		bool isSuccess;
		uint8_t x;
		uint8_t y;
		std::vector<uint8_t> coords;
	};

	Result cast(ClientCtxPtr pCc, uint8_t x, uint8_t y);

private:
	static std::pair<int, int> castRange[4];
};


class Enhancement {
public:
	struct Data {
		uint8_t x;
		uint8_t y;
	};

	struct Result {
		bool isSuccess;
	};

	Result cast(ClientCtxPtr pCc, uint8_t x, uint8_t y);
};


class Portal {
public:
	void cast() {}
};


class Ambush {
public:
	struct Data {
		uint8_t x1;
		uint8_t y1;
		uint8_t x2;
		uint8_t y2;
	};

	struct Result {
		bool isSuccess1;
		uint8_t x1;
		uint8_t y1;
		bool isSuccess2;
		uint8_t x2;
		uint8_t y2;
	};

	Result cast(ClientCtxPtr pCc, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
};


#endif