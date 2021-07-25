#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <Windows.h>
#include <iostream>
#include <cstdarg>
#include <tchar.h>
#include <ctime>

namespace Logger {
	void Log(uint8_t type, const char* inFormat, ...);
	std::string Sprintf(const char* inFormat, ...);
}

#define LOG_TYPE_NOTIFY	0
#define LOG_TYPE_ERROR	1
#define LOG_TYPE_DEBUG	2

#define LOG_NOTIFY(...)	Logger::Log(LOG_TYPE_NOTIFY, __VA_ARGS__);
#define LOG_ERROR(...)	Logger::Log(LOG_TYPE_ERROR, __VA_ARGS__);
#define LOG_DEBUG(...)	Logger::Log(LOG_TYPE_DEBUG, __VA_ARGS__);

#endif