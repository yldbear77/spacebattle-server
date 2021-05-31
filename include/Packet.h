#ifndef __PACKET_H__
#define __PACKET_H__

#include <cstdint>

constexpr int CS_REQ_CONNECT =	0;
constexpr int SC_RES_CONNECT =	1;

typedef uint16_t	PACKET_SIZE;
typedef uint8_t		PACKET_TYPE;

#pragma pack(push, 1)

typedef struct {
	PACKET_SIZE size;
	PACKET_TYPE type;
	uint8_t name[20];
	uint8_t character;
	uint8_t mapCode;
} csReqConnect;

typedef struct {
	PACKET_SIZE size;
	PACKET_TYPE type;
	uint8_t resCode;
} scReqConnect;

#pragma pack(pop)

#endif