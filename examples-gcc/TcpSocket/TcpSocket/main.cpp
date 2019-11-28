#include <cstdio>
#include <iostream>
#include <ryulib/TcpClient.hpp>

using namespace std;

int main()
{
	cout << "TcpClient Example\n";

	string lines = "";

	TcpClient socket;	
	socket.setOnConnected(
		[]() {
			printf("Connected. \n");
		}
	);	
	socket.setOnDisconnected(
		[&]() {
			printf("Disconnected. \n");
			printf("%s\n", lines.c_str());
			lines = "";
		}
	);
	socket.setOnReceived(
		[&](void* data, int size) {
			char* text = (char*) data;
			text[size] = 0x00;
			lines = lines + text;

			//printf("Received - size: %d \n", size);
			//printf("%s \n", text);
		}
	);
	socket.setOnError(
		[](int code, string msg) {
			printf("%s \n", msg.c_str());
		}
	);

	//char* host = "127.0.0.1";
	//int port = 22;

	char* host = "www.google.com";
	int port = 80;

	while (true) {
		int cmd;
		cin >> cmd;
		switch (cmd) {
			case 0: socket.connect(host, port); break;
			case 1: socket.disconnect(); break;
			case 2: socket.sendText("GET / HTTP/1.0 \n\n"); break;
			case 99: return 0;
		}
	}

	return 0;
}