#ifndef __PACKET_H__
#define __PACKET_H__

#include <cstdint>

constexpr int CS_REQ_CONNECT	= 0;
constexpr int SC_RES_CONNECT	= 1;
constexpr int SC_MATCH_SUCCESS	= 2;
constexpr int SC_REQ_DEPLOY		= 3;

typedef uint16_t	PACKET_SIZE;
typedef uint8_t		PACKET_TYPE;

#endif