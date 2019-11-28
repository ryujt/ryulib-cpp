#include <string>
#include <iostream>
#include <ryulib/SimpleThread.hpp>

using namespace std;

int main()
{
	SimpleThread thread([&](SimpleThread* simple_thread) {
		while (simple_thread->isTerminated() == false) {
			simple_thread->sleepTight();
			printf("Hello? \n");
		}

		printf("thread is stopped. \n");
	});

	while (true) {
		string line;
		printf("Command: ");
		getline(cin, line);

		if (line == "q") break;

		if (line == "t") thread.wakeUp();

		if (line == "t") {
			thread.terminate();
			break;
		}

		if (line == "tw") {
			thread.terminateAndWait();
			break;
		}

		if (line == "tn") {			
			thread.terminateNow();	
			break;
		}
	}

	printf("programm is about to close.");
}