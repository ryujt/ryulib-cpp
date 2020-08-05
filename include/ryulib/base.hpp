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
		data_ = nullptr;
		size_ = 0;
	}

	/** 
	������ ũ���� �޸𸮸� �Ҵ� ���� �����͸� ���� ��ü�� �����Ѵ�.
	@param size �Ҵ� ���� �޸��� ũ��
	*/
	Memory(int size)
	{
		size_ = size;
		if (size > 0) {
			data_ = malloc(size);
			memset(data_, 0, size);
		} else {
			data_ = nullptr;
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
			data_ = nullptr;
			size_ = 0;
			return;
		}

		size_ = size;
		data_ = malloc(size);
		memcpy(data_, data, size);
	}

	~Memory()
	{
		if (data_ != nullptr) {
			free(data_);
			data_ = nullptr;
		}
		size_ = 0;
	}

	/** 
	������ �����Ϳ��� �����͸� �����ؿ´�. ��ü�� size_ �Ӽ��� ������� �ʰ� �����͸� �����´�.
	@param src ������ �� �����Ͱ� �ִ� �޸� �ּ�
	@param size ������ �� �������� ũ��
	*/
	void loadMemory(const void* src, int size)
	{
		memcpy(data_, src, size);
	}

	/** 
	��ü ���ο� ����Ǿ� �ִ� �������� �޸� �ּ� */
	void* getData() { return data_; }

	/**
	��ü ���ο� ����Ǿ� �ִ� �������� ũ�� */
	int getSize() { return size_; }

	/**
	Memory ��ü�� �Բ� �����ϰ� ���� ������ �� ���� �� ����Ѵ�. */
	void* getUserData() { return user_data_; }

	/**
	Memory ��ü�� �Բ� �����ϰ� ���� ������ �� ���� �� ����Ѵ�. (�߰����� ������ ���ڷ� �����Ǵ� ���) */
	int getTag() { return tag_; }

	/**
	Memory ��ü�� �Բ� �����ϰ� ���� ������ �� ���� �� ����Ѵ�. (�߰����� ������ ���ڷ� �����Ǵ� ���) */
	string getText() { return text_; }

	void setUserData(const void* user_data) { user_data_ = (void*) user_data; }
	void setTag(int tag) { tag_ = tag; }
	void setText(string text) { text_ = text; }

private:
	void* data_ = nullptr;
	int size_ = 0;
	void* user_data_ = nullptr;
	int tag_ = 0;
	string text_;
};

}
#endif  // RYULIB_BASE_HPP
