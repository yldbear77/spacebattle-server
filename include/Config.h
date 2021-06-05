#ifndef __CONFIG_H__
#define __CONFIG_H__

//constexpr int SERVER_IP		= INADDR_ANY;
//constexpr int SERVER_PORT	= 9001;

#define SERVER_IP		INADDR_ANY
#define SERVER_PORT		9001

#define MAX_CLIENTS		50

#define MAX_BUF_SIZE	65540

constexpr int MODE_SEND =	0;
constexpr int MODE_RECV =	1;

constexpr int MAP_10_10	=	0;
constexpr int MAP_20_20	=	1;

#endif