#include "pch.h"
#include <iostream>
#include <ryulib/WaitFreeList.hpp>

class Test {
public:
	Test(int v)
		:value(v)
	{
	}

	int value;
};

int main()
{
	WaitFreeList<Test> list;

	list.add(new Test(1));
	list.add(new Test(2));
	list.add(new Test(3));

	Node *node = list.get_first();
	while (node != nullptr) {
		printf("%d \n", list.get_item(node).value);
		node = list.get_next(node);
	}
}