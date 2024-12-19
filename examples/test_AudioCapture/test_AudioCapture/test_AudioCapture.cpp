#include <iostream>
#include <WASAPI/AudioCapture.hpp>

int main()
{
    AudioCaptureOption option;
    option.mic_device_id = -1;
    option.use_system_audio = true;
    option.channels = 1;
    option.sample_rate = 48000;
    option.sample_size = 4;
    option.frames = 5760;

    AudioCapture audioCapture;

    int count = 0;
    audioCapture.setOnData([&](const void* sender, const void* data, int size) {
        printf("no: %d, size: %d \r", ++count, size);
    });

    if (audioCapture.start(option) == false) {
        printf("Error - audioCapture.start(option)");
        return -1;
    }

    while (true) {
        Pa_Sleep(1000);
    }
}
