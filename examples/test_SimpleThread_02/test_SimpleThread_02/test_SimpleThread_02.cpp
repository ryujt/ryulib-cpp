#include <string>
#include <iostream>
#include <ryulib/SimpleThread.hpp>

using namespace std;

int main()
{
	SimpleThread thread([&](SimpleThread* simple_thread) {
		while (true) {
			simple_thread->sleepTight();
			printf("Hello? \n");
		}
	});

	while (true) {
		string line;
		printf("Command: ");
		getline(cin, line);

		if (line == "q") break;

		thread.wakeUp();
	}
}