#ifndef RYULIB_BASE_HPP
#define RYULIB_BASE_HPP

#include <stdlib.h>
#include <string>
#include <cstring>
#include <functional>

using namespace std;

namespace ryulib {

class Memory;

typedef function<void()> VoidEvent;
typedef function<void(const void*)> NotifyEvent;
typedef function<void(const void*, const string)> StringEvent;
typedef function<void(const void*, int code, const string)> ErrorEvent;
typedef function<void(const void*, int)> IntegerEvent;
typedef function<void(const const const void*, Memory*)> MemoryEvent;
typedef function<void(const void*, const void*, int)> DataEvent;
typedef function<bool(const void*)> AskEvent;
typedef function<void(int, const string, const void*, int, int)> TaskEvent;

/** 
메모리 할당을 받아서 포인터와 크기를 함께 묶어서 사용하는 데이터 클래스 */
class Memory {
public:
	/**
	크기가 0인 null 포인터를 갖는 객체를 생성한다. */
	Memory()
	{
		_data = nullptr;
		_size = 0;
	}

	/**
	지정된 크기의 메모리를 할당 받은 포인터를 갖는 객체를 생성한다.
	@param size 할당 받을 메모리의 크기
	@param fill_zero 할당 받은 공간 메모리를 0으로 초기화 할 것인가?
	*/
	Memory(int size, bool fill_zero = true)
	{
		_size = size;
		if (size > 0) {
			_data = malloc(size);
			if (fill_zero) memset(_data, 0, size);
		}
		else {
			_data = nullptr;
		}
	}

	/**
	지정된 포인터로부터 데이터를 복사해서 데이터를 저장하는 객체를 생성한다.
	@param data 복사해 올 데이터가 있는 메모리 주소
	@param size 복사해 올 데이터의 크기
	*/
	Memory(const void* data, int size)
	{
		if ((size <= 0) || (data == nullptr)) {
			_data = nullptr;
			_size = 0;
			return;
		}

		_size = size;
		_data = malloc(size);
		memcpy(_data, data, size);
	}

	~Memory()
	{
		if (_data != nullptr) {
			free(_data);
			_data = nullptr;
		}
		_size = 0;
	}

	/**
	지정된 포인터에서 데이터를 복사해온다. 객체의 _size 속성은 변경되지 않고 데이터만 가져온다.
	@param src 복사해 올 데이터가 있는 메모리 주소
	@param size 복사해 올 데이터의 크기
	*/
	void loadMemory(const void* src, int size)
	{
		memcpy(_data, src, size);
	}

	/**
	객체 내부에 저장되어 있는 데이터의 메모리 주소 */
	void* getData() { return _data; }

	/**
	객체 내부에 저장되어 있는 데이터의 크기 */
	int getSize() { return _size; }

	/**
	Memory 객체에 함께 저장하고 싶은 정보가 더 있을 때 사용한다. */
	void* getUserData() { return _userData; }

	/**
	Memory 객체에 함께 저장하고 싶은 정보가 더 있을 때 사용한다. (추가적인 정보가 숫자로 구별되는 경우) */
	int getTag() { return _tag; }

	/**
	Memory 객체에 함께 저장하고 싶은 정보가 더 있을 때 사용한다. (추가적인 정보가 문자로 구별되는 경우) */
	string getText() { return _text; }

	void setUserData(const void* userData) { _userData = (void*)userData; }
	void setTag(int tag) { _tag = tag; }
	void setText(string text) { _text = text; }

private:
	void* _data = nullptr;
	int _size = 0;
	void* _userData = nullptr;
	int _tag = 0;
	string _text;
};

}
#endif  // RYULIB_BASE_HPP
