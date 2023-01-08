#include <iostream>
#include <WASAPI/AudioCapture.hpp>

int main()
{
    AudioCaptureOption option;
    option.mic_device_id = -1;
    option.use_system_audio = true;
    option.channels = 1;
    option.sample_rate = 44100;
    option.sample_size = 4;
    option.frames = 1024;

    AudioCapture audioCapture;

    audioCapture.setOnData([&](const void* sender, const void* data, int size) {
        printf("size: %d \r", size);
        //delete data;
    });

    audioCapture.start(option);

    while (true) {
        // 
    }
}
