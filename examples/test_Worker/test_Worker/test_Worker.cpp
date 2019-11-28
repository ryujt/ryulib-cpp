#include <string>
#include <iostream>
#include <ryulib/Worker.hpp>

int main()
{
	Worker worker;
	worker.setOnString([](string text) {
		printf("OnString: %s \n", text.c_str());
	});

	while (true) {
		string line;
		printf("Command: ");
		getline(cin, line);

		worker.add(line);
	}
}