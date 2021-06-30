#ifndef __PACKET_H__
#define __PACKET_H__

#include <cstdint>

/* packet type */
constexpr int CS_REQ_CONNECT	= 0;
constexpr int SC_RES_CONNECT	= 1;
constexpr int SC_MATCH_SUCCESS	= 2;
constexpr int SC_REQ_DEPLOY		= 3;
constexpr int CS_RES_DEPLOY		= 4;

/* response code */
constexpr int RES_SUCCESS		= 0;
constexpr int RES_FAILED		= 1;

typedef uint16_t	PACKET_SIZE;
typedef uint8_t		PACKET_TYPE;

#endif