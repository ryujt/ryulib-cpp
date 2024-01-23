#pragma once

#include <iostream>
#include <string>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <locale>
#include <codecvt>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class UdpSocket {
public:
    static UdpSocket& getInstance() {
        static UdpSocket instance;
        return instance;
    }

    UdpSocket(UdpSocket const&) = delete;
    void operator=(UdpSocket const&) = delete;

    void setAddress(string ip, int port) {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr));
    }

    void sendText(string text) {
        sendto(sock, text.c_str(), text.size(), 0, (const sockaddr*)&addr, sizeof(addr));
    }

    void sendText(wstring text) {
        sendText(string(text.begin(), text.end()));
    }

private:
    UdpSocket() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw runtime_error("Failed to initialize winsock");
        }

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET) {
            WSACleanup();
            throw runtime_error("Failed to create socket");
        }

        memset(&addr, 0, sizeof(addr));
    }

    ~UdpSocket() {
        closesocket(sock);
        WSACleanup();
    }

    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in addr;
};
