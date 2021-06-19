#include "Logger.h"

void Logger::Log(uint8_t type, const char* inFormat, ...) {
	static char temp[4096]; // not thread safe...
	static wchar_t msg[6000];

	va_list args;
	va_start(args, inFormat);

	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80] = { 0 };
	localtime_s(&tstruct, &now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct); // YYYY-MM-DD.HH:mm:ss 형태의 스트링

	_vsnprintf_s(temp, 4096, 4096, inFormat, args);

	size_t len = strlen(temp) + 1;
	mbstowcs_s(&len, msg, temp, 6000);

	switch (type) {
	case LOG_TYPE_NOTIFY:
		printf("[%s]%-8s %S\n", buf, "[NOTIFY]", msg);
		break;
	case LOG_TYPE_ERROR:
		printf("[%s]%-8s %S\n", buf, "[ERROR]", msg);
		break;
	case LOG_TYPE_DEBUG:
		OutputDebugString(msg);
		break;
	}
}

std::string Logger::Sprintf(const char* inFormat, ...) {
	static char temp[4096]; //not thread safe...

	va_list args;
	va_start(args, inFormat);

	_vsnprintf_s(temp, 4096, 4096, inFormat, args);
	return std::string(temp);
}