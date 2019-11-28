#include <iostream>
#include <ryulib/SimpleThread.hpp>

using namespace std;

int count = 0;

class Test {
public:
	Test() {
		thread_ = new SimpleThread(thread_OnExecute);
	}

	void stop() {
		printf("stop()\n");
	}

private:
	SimpleThread *thread_;
	SimpleThreadEvent thread_OnExecute = [](SimpleThread *thread) {
		printf("thread_OnExecute()\n");

		while (thread->isTerminated() == false) {
			printf("Hi\n");
			thread->Sleep(100);
		}
	};
};


int main()
{
	Test *test = new Test();
	getchar();
	test->stop();

	return 0;
}
