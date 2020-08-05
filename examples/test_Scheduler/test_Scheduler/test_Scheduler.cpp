#include <string>
#include <iostream>
#include <ryulib/Scheduler.hpp>

const int TASK_CONNECT = 1;
const int TASK_DISCONNECT = 2;

class Address {
public:
	Address(string ip, int port)
		: ip_(ip), port_(port)
	{		
	}
	string ip_;
	int port_;
};

int main()
{
	Scheduler scheduler;
	scheduler.setOnTask([](int task, const string text, const void* data, int size, int tag) {
		switch (task) {
			case TASK_CONNECT: {
				Address* address = (Address*) data;
				printf("Connect to %s:%d \n", address->ip_.c_str(), address->port_);
				delete address;
				break;
			}

			case TASK_DISCONNECT: {
				printf("Disconnect \n");
				break;
			}
		}
	});
	scheduler.setOnRepeat([&](){
		printf("수신된 메시지 확인... \n");
		scheduler.sleep(1000);
	});
	scheduler.start();

	while (true) {
		string line;
		printf("Command: ");
		getline(cin, line);

		if (line == "c") scheduler.add(TASK_CONNECT, "", new Address("127.0.0.1", 1234), 0, 0);
		if (line == "d") scheduler.add(TASK_DISCONNECT);
	}
}