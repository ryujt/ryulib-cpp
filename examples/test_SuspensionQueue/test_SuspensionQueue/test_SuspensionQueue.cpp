#include <string>
#include <iostream>
#include <ryulib/SuspensionQueue.hpp>
#include <ryulib/SimpleThread.hpp>

using namespace std;

int main()
{
	SuspensionQueue<string> que;

	SimpleThread producer([&](SimpleThread* simple_thread) {
		while (simple_thread->isTerminated() == false) {
			que.push("task");
			printf("task has produced. \n");
			simple_thread->sleep(1000);
		}
	});

	SimpleThread consumer([&](SimpleThread* simple_thread) {
		while (simple_thread->isTerminated() == false) {
			string item = que.pop();
			item = item + " --> used ";
			printf("%s \n", item.c_str());
		}
	});


	while (true) {
		Sleep(1000);
	}
}