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
		size_ = 0;
	}

	void *getData() { return data_; }

	int getSize() { return size_; }

private:
	void* data_ = nullptr;
	int size_ = 0;
};

#endif  // RYULIB_BASE_HPP
