#include <string>
#include <iostream>
#include <ryulib/AudioZip.hpp>
#include <ryulib/AudioUnZip.hpp>

using namespace std;

int main(void) {
	Audio::init();

	AudioUnZip unzip(1, 48000);
	unzip.setOnError([&](const void* obj, int error_code) {
		printf("AudioUnZip - error: %d", error_code);
	});

	AudioZip zip(1, 48000);
	zip.setOnError([&](const void* obj, int error_code) {
		printf("AudioZip - error: %d", error_code);
	});
	zip.setOnEncode([&](const void* obj, const void* data, int size) {
		unzip.play(data, size);
	});

	while (true) {
		printf("(s)tart, s(t)op: ");
		string line;
		getline(cin, line);

		if (line == "s") zip.start();
		if (line == "t") zip.stop();
	}
}