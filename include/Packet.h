#ifndef __PACKET_H__
#define __PACKET_H__

#include <cstdint>

/* packet type */
constexpr int CS_REQ_CONNECT	= 0;
constexpr int SC_RES_CONNECT	= 1;

constexpr int SC_MATCH_SUCCESS	= 2;

constexpr int SC_REQ_DEPLOY		= 3;
constexpr int CS_RES_DEPLOY		= 4;

constexpr int SC_DEPLOY_RESULT	= 5;
constexpr int SC_START_BP		= 6;

constexpr int CS_REQ_ATTACK		= 7;
constexpr int CS_REQ_SKILL		= 8;
constexpr int SC_RES_ATTACK		= 9;
constexpr int SC_RES_SKILL		= 10;

/* response code */
constexpr int RES_SUCCESS		= 0;
constexpr int RES_FAILED		= 1;

constexpr int ALREADY_DESTROYED	= 2;

typedef uint16_t	PACKET_SIZE;
typedef uint8_t		PACKET_TYPE;

#endif