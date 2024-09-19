#include <string>
#include <iostream>
#include <ryulib/AudioZip.hpp>
#include <ryulib/AudioUnZip.hpp>

using namespace std;

int main(void) {
	Audio::init();

	AudioUnZip unzip(1, 48000);
	unzip.setOnError([&](const void* obj, int error_code) {
		printf("AudioUnZip - error: %d \n", error_code);
	});

	int count = 0;

	AudioZip zip(1, 48000);
	zip.setOnError([&](const void* obj, int error_code) {
		printf("AudioZip - error: %d \n", error_code);
	});
	zip.setOnSource([&](const void* obj, const void* data, int size) {
		printf("OnSource - %d, size: %d, ", ++count, size);
	});
	zip.setOnEncode([&](const void* obj, const void* data, int size) {
		printf("OnEncode - size: %d \r", size);
		unzip.play(data, size);
	});

	while (true) {
		printf("(s)tart, s(t)op: ");
		string line;
		getline(cin, line);

		if (line == "s") zip.start(-1);
		if (line == "t") zip.stop();
	}
}