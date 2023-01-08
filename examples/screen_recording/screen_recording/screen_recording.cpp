#include "FFmpegController.hpp"
#include <Windows.h>
#include <iostream>
#include <string>
#include <ryulib/PipeSender.hpp>
#include <WASAPI/AudioCapture.hpp>
#include <DeskZip/DesktopCapture.hpp>
#include <ryulib/strg.hpp>

int main()
{
	string options = "{"
		"\"rid\": \"1234\", "
		"\"mic\" : -1, "
		"\"system-audio\" : true, "
		"\"volume-mic\" : 1.0, "
		"\"volume-system\" : 1.0, "
		"\"speed\" : \"veryfast\", "
		"\"left\" : 0, "
		"\"top\" : 0, "
		"\"width\" : 1920, "
		"\"height\" : 1080, "
		"\"with-cursor\" : true"
		"}";

	FFmpegController controller;
	controller.open(options.c_str());

	while (true) {
		string line;
		printf("(s)top: ");
		getline(cin, line);

		if (line == "s") {
			controller.close();
			return 0;
		}
	}
}
