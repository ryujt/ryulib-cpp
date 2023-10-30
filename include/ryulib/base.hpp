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
�޸� �Ҵ��� �޾Ƽ� �����Ϳ� ũ�⸦ �Բ� ��� ����ϴ� ������ Ŭ���� */
class Memory {
public:
	/**
	ũ�Ⱑ 0�� null �����͸� ���� ��ü�� �����Ѵ�. */
	Memory()
	{
		_data = nullptr;
		_size = 0;
	}

	/**
	������ ũ���� �޸𸮸� �Ҵ� ���� �����͸� ���� ��ü�� �����Ѵ�.
	@param size �Ҵ� ���� �޸��� ũ��
	@param fill_zero �Ҵ� ���� ���� �޸𸮸� 0���� �ʱ�ȭ �� ���ΰ�?
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
	������ �����ͷκ��� �����͸� �����ؼ� �����͸� �����ϴ� ��ü�� �����Ѵ�.
	@param data ������ �� �����Ͱ� �ִ� �޸� �ּ�
	@param size ������ �� �������� ũ��
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
	������ �����Ϳ��� �����͸� �����ؿ´�. ��ü�� _size �Ӽ��� ������� �ʰ� �����͸� �����´�.
	@param src ������ �� �����Ͱ� �ִ� �޸� �ּ�
	@param size ������ �� �������� ũ��
	*/
	void loadMemory(const void* src, int size)
	{
		memcpy(_data, src, size);
	}

	/**
	��ü ���ο� ����Ǿ� �ִ� �������� �޸� �ּ� */
	void* getData() { return _data; }

	/**
	��ü ���ο� ����Ǿ� �ִ� �������� ũ�� */
	int getSize() { return _size; }

	/**
	Memory ��ü�� �Բ� �����ϰ� ���� ������ �� ���� �� ����Ѵ�. */
	void* getUserData() { return _userData; }

	/**
	Memory ��ü�� �Բ� �����ϰ� ���� ������ �� ���� �� ����Ѵ�. (�߰����� ������ ���ڷ� �����Ǵ� ���) */
	int getTag() { return _tag; }

	/**
	Memory ��ü�� �Բ� �����ϰ� ���� ������ �� ���� �� ����Ѵ�. (�߰����� ������ ���ڷ� �����Ǵ� ���) */
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
