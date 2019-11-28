#include <string>
#include <iostream>
#include <ryulib/ThreadQueue.hpp>
#include <ryulib/SimpleThread.hpp>

using namespace std;

int main()
{
	ThreadQueue<string> que;

	SimpleThread producer([&](SimpleThread* simple_thread) {
		while (simple_thread->isTerminated() == false) {
			que.push("task");
			printf("task has produced. \n");
			simple_thread->sleep(1000);
		}
	});

	SimpleThread consumer([&](SimpleThread* simple_thread) {
		while (simple_thread->isTerminated() == false) {
			string item = "";
			if (que.pop(item)) {
				item = item + " --> used ";
				printf("%s \n", item.c_str());
			} else {
				simple_thread->sleep(1);
			}
		}
	});


	while (true) {
		Sleep(1000);
	}
}