#include "pch.h"

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

static void InitializeWinsock()
{
    static bool initialized = false;
    if (!initialized)
    {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) return;
        initialized = true;
    }
}

extern "C" __declspec(dllexport) void SendToUDP(const wchar_t* host, int port, const wchar_t* text)
{
    InitializeWinsock();
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET) return;

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (InetPtonW(AF_INET, host, &serverAddr.sin_addr) != 1)
    {
        closesocket(udpSocket);
        return;
    }

    int len = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
    std::string utf8Text(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, (char*)utf8Text.data(), len, NULL, NULL);

    sendto(udpSocket, utf8Text.c_str(), (int)utf8Text.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    closesocket(udpSocket);
}
