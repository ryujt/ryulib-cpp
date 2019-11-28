#include <iostream>
#include <ryulib/SimpleThread.hpp>

int main()
{
	SimpleThread thread([&](SimpleThread* simple_thread) {
		while (true) {
			printf("Hello from thread. \n");
			simple_thread->sleep(1000);
		}
	});

	while (true) {
		printf("Hello from main. \n");
		Sleep(1000);
	}
}