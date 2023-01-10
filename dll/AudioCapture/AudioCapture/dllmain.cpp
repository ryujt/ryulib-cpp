// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <ryulib/AudioIO.hpp>
#include <ryulib/AudioZipUtils.hpp>
#include <WASAPI/AudioCapture.hpp>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

typedef void (*CALLBACK_DATA) (const void* sender, const void*, int);
typedef void (*CALLBACK_ERROR) (const void* sender, int);

class AudioCaptureHandle {
private:
	void* _context;

	CALLBACK_DATA _callback_on_data;
	CALLBACK_ERROR _callback_on_error;

	DataEvent _on_data = [&](const void* obj, const void* data, int size) {
		_callback_on_data(_context, data, size);
	};

	IntegerEvent _on_error = [&](const void* obj, int error_code) {
		_callback_on_error(_context, error_code);
	};

public:
	AudioCapture* object;

	AudioCaptureHandle(void* context, CALLBACK_DATA on_data, CALLBACK_ERROR on_error)
		: _context(context), _callback_on_data(on_data), _callback_on_error(on_error)
	{
		object = new AudioCapture();
		object->setOnData(_on_data);
		object->setOnError(_on_error);
	}

	~AudioCaptureHandle()
	{
		delete object;
	}
};

extern "C" __declspec(dllexport) void initAudioZip()
{
	Audio::init();
}

extern "C" __declspec(dllexport) AudioCaptureHandle * createAudioCapture(void* context, CALLBACK_DATA on_data, CALLBACK_ERROR on_error)
{
	return new AudioCaptureHandle(context, on_data, on_error);
}

extern "C" __declspec(dllexport) bool startAudioCapture(AudioCaptureHandle * handle, int device_id, bool use_system_audio)
{
	AudioCaptureOption option;
	option.mic_device_id = device_id;
	option.sample_rate = SAMPLE_RATE;
	option.frames = FRAMES_PER_BUFFER;
	option.use_system_audio = use_system_audio;
	return handle->object->start(option);
}

extern "C" __declspec(dllexport) void stopAudioCapture(AudioCaptureHandle * handle)
{
	handle->object->stop();
}

extern "C" __declspec(dllexport) bool isMicMuted(AudioCaptureHandle * handle)
{
	return handle->object->isMicMuted();
}

extern "C" __declspec(dllexport) bool isSystemMuted(AudioCaptureHandle * handle)
{
	return handle->object->isSystemMuted();
}

extern "C" __declspec(dllexport) float getMicVolume(AudioCaptureHandle * handle)
{
	return handle->object->getMicVolume();
}

extern "C" __declspec(dllexport) float getSystemVolume(AudioCaptureHandle * handle)
{
	return handle->object->getSystemVolume();
}

extern "C" __declspec(dllexport) void setMicMute(AudioCaptureHandle * handle, bool value)
{
	handle->object->setMicMute(value);
}

extern "C" __declspec(dllexport) void setSystemMute(AudioCaptureHandle * handle, bool value)
{
	handle->object->setSystemMute(value);
}

extern "C" __declspec(dllexport) void setMicVolume(AudioCaptureHandle * handle, float volume)
{
	handle->object->setMicVolume(volume);
}

extern "C" __declspec(dllexport) void setSystemVolume(AudioCaptureHandle * handle, float volume)
{
	handle->object->setSystemVolume(volume);
}

extern "C" __declspec(dllexport) void releaseAudioZip(AudioCaptureHandle * handle)
{
	handle->object->setOnError(nullptr);
	handle->object->setOnData(nullptr);
	delete handle->object;
	delete handle;
}