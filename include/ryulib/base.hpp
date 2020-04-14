#ifndef RYULIB_BASE_HPP
#define RYULIB_BASE_HPP

#include <stdlib.h>
#include <string>
#include <cstring>
#include <functional>

using namespace std;

class Memory;

typedef function<void()> VoidEvent;
typedef function<void(const void*)> NotifyEvent;
typedef function<void(const void*, const string)> StringEvent;
typedef function<void(const void*, int code, const string)> ErrorEvent;
typedef function<void(const void*, int)> IntegerEvent;
typedef function<void(const void*, const Memory*)> MemoryEvent;
typedef function<void(const void*, const void*, int)> DataEvent;
typedef function<bool(const void*)> AskEvent;
typedef function<void(int, const string, const void*, int, int)> TaskEvent;

class Memory {
public:
	Memory()
	{
		data_ = nullptr;
		size_ = 0;
	}

	Memory(int size)
	{
		size_ = size;
		if (size > 0) {
			data_ = malloc(size);
		}
		else {
			data_ = nullptr;
		}
	}

	Memory(const void* data, int size)
	{
		size_ = size;
		if (size > 0) {
			data_ = malloc(size);
			memcpy(data_, data, size);
		}
		else {
			data_ = nullptr;
		}
	}

	~Memory()
	{
		if (data_ != nullptr) {
			free(data_);
			data_ = nullptr;
		}
	}

	void *getData() { return data_; }

	int getSize() { return size_; }

private:
	void* data_ = nullptr;
	int size_ = 0;
};

#pragma pack(push, 1)
typedef struct _Packet {
	unsigned short packet_size;
	char packet_type;
	char data_start;

	int getDataSize() { return packet_size - 3; }

	char* getText()
    {
	    char* result = (char*) malloc(packet_size);
	    memset(result, 0, packet_size);
	    memcpy(result, &data_start, getDataSize());
	    return result;
    }
} Packet;
#pragma pack(pop)

static Packet* create_packet(char packet_type, const void* data, int size)
{
	Packet* packet = (Packet*) malloc(size + sizeof(Packet) - 1);
	if (packet == NULL) return nullptr;

	packet->packet_type = packet_type;
	packet->packet_size = size + sizeof(Packet) - 1;
	if (size > 0) memcpy(&packet->data_start, data, size);

	return packet;
}

static Packet* create_packet(char packet_type, string text)
{
    Packet* packet = (Packet*) malloc(text.size() + sizeof(Packet) - 1);
    if (packet == NULL) return nullptr;

    packet->packet_type = packet_type;
    packet->packet_size = text.size() + sizeof(Packet) - 1;
	if (text.size() > 0) memcpy(&packet->data_start, text.c_str(), text.size());

    return packet;
}

#endif  // RYULIB_BASE_HPP
