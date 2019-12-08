#include <stdio.h>
#include <ryulib/SocketUtils.hpp>
#include <ryulib/SocketServer.hpp>

int main()
{
	SocketServer server;

	server.setOnConnected([](Connection* connection){
		printf("Connected! \n");
	});

	server.setOnDisconnected([](Connection* connection){
		printf("Disconnected! \n");
	});

	server.setOnError([](int code, string msg){
		printf("Error - %d, %s \n", code, msg.c_str());
	});

	server.setOnReceived([&](Connection *connection, Packet* packet){
		printf("Received - %d, %s \n", packet->packet_type, packet->getString().c_str());
		server.sendToAll(packet);
	});

	server.start(1234);
}
