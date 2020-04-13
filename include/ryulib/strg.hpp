#ifndef _RYULIB_STRG_H_
#define	_RYULIB_STRG_H_

#ifdef _WIN32
#include "windows.h"
#endif

#include <string>

#ifdef _WIN32
static char *WideCharToChar(wchar_t *src)
{
	int len = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
	char *result = new char[len];
	WideCharToMultiByte(CP_ACP, 0, src, -1, result, len, 0, 0);
	return result;
}
#endif

template<typename ... Args>
static std::string format_string(const std::string& format, Args ... args)
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
	std::unique_ptr<char[]> buffer(new char[size]);
	snprintf(buffer.get(), size, format.c_str(), args ...);
	return std::string(buffer.get(), buffer.get() + size - 1);
}

#endif // _RYULIB_STRG_H_ 