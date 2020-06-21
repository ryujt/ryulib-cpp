#ifndef _RYULIB_STRG_H_
#define	_RYULIB_STRG_H_

#ifdef _WIN32
#include "windows.h"
#endif

#include <string>
#include <algorithm>

using namespace std;

#ifdef _WIN32
static char* WideCharToChar(wchar_t* src)
{
    int len = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
    char* result = new char[len];
    WideCharToMultiByte(CP_ACP, 0, src, -1, result, len, 0, 0);
    return result;
}

static string WideCharToString(wchar_t* src)
{
    char* temp = WideCharToChar(src);
    string result = string(temp);
    delete temp;
    return result;
}

static wchar_t* StringToWideChar(string str)
{
    int len;
    int slength = str.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
    wchar_t* result = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, result, len);
    return result;
}
#endif

template<typename ... Args>
static string format_string(const string& format, Args ... args)
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
	auto buffer(new char[size]);
	snprintf(buffer.get(), size, format.c_str(), args ...);
	return string(buffer.get(), buffer.get() + size - 1);
}

static string deleteLeft(string str, string border, bool ignore_case = false)
{
    string src = string(str);
    string dst = string(border);

    if (ignore_case) {
        transform(src.begin(), src.end(), src.begin(), ::tolower);
        transform(dst.begin(), dst.end(), dst.begin(), ::tolower);
    }

    int pos = src.find(dst);
    if (pos <= 0) {
        return str;
    }

    return str.substr(pos, str.length());
}

static string deleteLeftPlus(string str, string border, bool ignore_case = false)
{
    string src = string(str);
    string dst = string(border);

    if (ignore_case) {
        transform(src.begin(), src.end(), src.begin(), ::tolower);
        transform(dst.begin(), dst.end(), dst.begin(), ::tolower);
    }

    int pos = src.find(dst);
    if (pos < 0) {
        return str;
    }

    return str.substr(pos + border.length(), str.length());
}

static string deleteRight(string str, string border, bool ignore_case = false)
{
    string src = string(str);
    string dst = string(border);

    if (ignore_case) {
        transform(src.begin(), src.end(), src.begin(), ::tolower);
        transform(dst.begin(), dst.end(), dst.begin(), ::tolower);
    }

    for (int i = src.length() - 1; i >= 0; i--) {
        if (src.substr(i, border.length()) == border) {
            return str.substr(0, i + border.length());
        }
    }

    return str;
}

static string deleteRightPlus(string str, string border, bool ignore_case = false)
{
    string src = string(str);
    string dst = string(border);

    if (ignore_case) {
        transform(src.begin(), src.end(), src.begin(), ::tolower);
        transform(dst.begin(), dst.end(), dst.begin(), ::tolower);
    }

    for (int i = src.length() - 1; i >= 0; i--) {
        if (src.substr(i, border.length()) == border) {
            return str.substr(0, i);
        }
    }

    return str;
}

static string setLastString(string str, string last)
{
    if (str.substr(str.length() - last.length(), last.length()) == last) {
        return str;
    } else {
        return str = str + last;
    }
}


#endif // _RYULIB_STRG_H_ 