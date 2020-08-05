#include <string>
#include <iostream>
#include <ryulib/Worker.hpp>

int main()
{
	Worker worker;
	worker.setOnTask([](int tast, const string text, const void* data, int size, int tag) {
		printf("OnString: %s \n", text.c_str());
	});

	while (true) {
		string line;
		printf("Command: ");
		getline(cin, line);

		worker.add(line);
	}
}