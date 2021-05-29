#ifndef __PACKET_H__
#define __PACKET_H__

#include <cstdint>

#pragma pack(push, 1)

typedef uint16_t		PACKET_SIZE;

typedef struct {
	PACKET_SIZE size;
	uint8_t name[20];
	uint8_t character;
	uint8_t mapCode;
} CS_REQ_CONNECT;

typedef struct {
	PACKET_SIZE size;
	uint8_t resCode;
} SC_RES_CONNECT;

#pragma pack(pop)

#endif