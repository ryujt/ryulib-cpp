#include <iostream>
#include <string>
#include <ryulib/SocketClient.hpp>

using namespace std;

int main()
{
	SocketClient socket;

	socket.setOnConnected([](){
		printf("Connected! \n");
	});

	socket.setOnDisconnected([](){
		printf("Disconnected! \n");
	});

	socket.setOnError([](int code, string msg){
		printf("Error - %d, %s \n", code, msg.c_str());
	});

	socket.setOnReceived([](Packet* packet){
		printf("Received: %s \n", packet->getText());
	});

	socket.connect("127.0.0.1", 1234);
    
	while (true) {
		string line;
		getline(cin, line);

		socket.send(0, line);
	}
}
