#pragma once

#include "StartOptions.hpp"
#include <ryulib/PipeSender.hpp>
#include <ryulib/strg.hpp>
#include <ryulib/debug_tools.hpp>
#include <WASAPI/AudioCapture.hpp>
#include <DeskZip/DesktopCapture.hpp>

using namespace std;

class FFmpegController {
public:
	~FFmpegController()
	{
		if (video_buffer != nullptr) {
			free(video_buffer);
			video_buffer = nullptr;
		}
	}

	bool open(const char* json)
	{
		DebugOutput::trace("open_ffmpeg_controller - %s", json);

		if (options.parseJson(string(json)) == false) return false;

		option_desktop.setTargetHandle(options.get_int("window-handle", -1));
		option_desktop.fps = 20;
		option_desktop.setLeft(options.get_int("left", 0));
		option_desktop.setTop(options.get_int("top", 0));
		option_desktop.setWidth(options.get_int("width", 0));
		option_desktop.setHeight(options.get_int("height", 0));
		option_desktop.with_cursor = options.get_bool("with-cursor", false);

		// TODO: 매직 넘버 제거
		option_audio.mic_device_id = options.get_int("mic", -1);
		option_audio.use_system_audio = options.get_bool("system-audio", false);
		option_audio.channels = 1;
		option_audio.sample_rate = 44100;
		option_audio.sample_size = 4;
		option_audio.frames = option_audio.sample_rate / option_desktop.fps;

		video_buffer = malloc(option_desktop.getBitmapSize());
		memset(video_buffer, 0, option_desktop.getBitmapSize());

		audioCapture.setOnData([&](const void* sender, const void* data, int size) {
			audio_sender.send(data, size);
			video_sender.send(video_buffer, option_desktop.getBitmapSize());
		});
		if (audioCapture.start(option_audio) == false) {
			free(video_buffer);
			return false;
		}

		desktopCapture.setOnData([&](const void* sender, const void* data, int size) {
			memcpy(video_buffer, data, size);
		});
		desktopCapture.start(option_desktop);

		string audio_pipe = "\\\\.\\pipe\\audio-" + options.get_string("rid", "");
		string video_pipe = "\\\\.\\pipe\\video-" + options.get_string("rid", "");

		audio_sender.open(StringToWideChar(audio_pipe));
		video_sender.open(StringToWideChar(video_pipe));

		return true;
	}

	void close()
	{
		DebugOutput::trace("close_ffmpeg_controller - audioCapture.stop()");
		audioCapture.stop();

		DebugOutput::trace("close_ffmpeg_controller - desktopCapture.stop()");
		desktopCapture.stop();

		DebugOutput::trace("close_ffmpeg_controller - audio_sender.stop()");
		audio_sender.close();

		DebugOutput::trace("close_ffmpeg_controller - video_sender.stop()");
		video_sender.close();

		if (video_buffer != nullptr) {
			free(video_buffer);
			video_buffer = nullptr;
		}
	}

private:
	void* video_buffer = nullptr;

	StartOption options;

	PipeSender audio_sender;
	PipeSender video_sender;

	DesktopCaptureOption option_desktop;
	AudioCaptureOption option_audio;

	DesktopCapture desktopCapture;
	AudioCapture audioCapture;
};
