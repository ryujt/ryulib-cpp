#ifndef RYULIB_SOCKETUTILS_HPP
#define RYULIB_SOCKETUTILS_HPP

#include <string>
#include <functional>

const int HEADER_SIZE = 3;
const int PACKET_LIMIT = 32768;
const int MAX_CONNECTION = 4096;

const int ERROR_CONNECT = -1;
const int ERROR_READ = -2;
const int ERROR_WRITE = -3;

using namespace std;

typedef function<void(int, string)> SocketErrorEvent;

#pragma pack(push,1)

typedef struct _PacketHeader {
	unsigned short packet_size;
	char packet_type;
} PacketHeader;

typedef struct _Packet {
	unsigned short packet_size;
	char packet_type;
	char data_start;

	void* getData() { return &data_start; }
	int getDataSize() { return packet_size - sizeof(PacketHeader); }

	string getString() 
	{
		return string(&data_start, getDataSize());
	}

} Packet;

#pragma pack(pop)

static Packet* create_packet(char packet_type, const void* data, int size)
{
	Packet* packet = (Packet*) malloc(size + sizeof(PacketHeader));
	if (packet == NULL) return nullptr;

	packet->packet_type = packet_type;
	packet->packet_size = size + sizeof(PacketHeader);
	memcpy(&packet->data_start, data, size);

	return packet;
}

#endif  // RYULIB_SOCKETUTILS_HPP