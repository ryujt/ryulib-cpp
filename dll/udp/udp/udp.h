#pragma once

#include <Windows.h>
#include <winsock2.h>
#include <string>

#ifdef UDP_EXPORTS
#define UDP_API __declspec(dllexport)
#else
#define UDP_API __declspec(dllimport)
#endif

extern "C" {
	UDP_API void SendToUDP(const wchar_t* host, int port, const wchar_t* text);
}
