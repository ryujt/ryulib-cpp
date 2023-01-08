// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"
#include <ryulib/AudioZip.hpp>
#include <ryulib/AudioUnZip.hpp>
#include <ryulib/debug_tools.hpp>

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

class AudioZipHandle {
private:
	void* context_;

	CALLBACK_DATA callback_on_source_;
	CALLBACK_DATA callback_on_encode_;
	CALLBACK_ERROR callback_on_error_;

	DataEvent on_source_ = [&](const void* obj, const void* data, int size) {
		callback_on_source_(context_, data, size);
	};

	DataEvent on_encode_ = [&](const void* obj, const void* data, int size) {
		callback_on_encode_(context_, data, size);
	};

	IntegerEvent on_error_ = [&](const void* obj, int error_code) {
		callback_on_error_(context_, error_code);
	};

public:
	AudioZip* object;

	AudioZipHandle(void* context, CALLBACK_DATA on_source, CALLBACK_DATA on_encode, CALLBACK_ERROR on_error)
		: context_(context), callback_on_source_(on_source), callback_on_encode_(on_encode), callback_on_error_(on_error)
	{
		object = new AudioZip(CHANNEL, SAMPLE_RATE);
		object->setOnSource(on_source_);
		object->setOnEncode(on_encode_);
		object->setOnError(on_error_);
	}

	~AudioZipHandle()
	{
		delete object;
	}
};

class AudioUnZipHandle {
private:
	void* context_;

	CALLBACK_ERROR callback_on_error_;

	IntegerEvent on_error_ = [&](const void* obj, int error_code) {
		callback_on_error_(context_, error_code);
	};

public:
	AudioUnZip* object;

	AudioUnZipHandle(void* context, CALLBACK_ERROR on_error)
		: context_(context), callback_on_error_(on_error)
	{
		object = new AudioUnZip(CHANNEL, SAMPLE_RATE);
		object->setOnError(on_error_);
	}

	~AudioUnZipHandle()
	{
		delete object;
	}
};

extern "C" __declspec(dllexport) void initAudioZip()
{
	Audio::init();
}

extern "C" __declspec(dllexport) AudioZipHandle* createAudioZip(void* context, CALLBACK_DATA on_source, CALLBACK_DATA on_encode, CALLBACK_ERROR on_error)
{
	return new AudioZipHandle(context, on_source, on_encode, on_error);
}

extern "C" __declspec(dllexport) bool startAudioZip(AudioZipHandle* handle, int device_id, bool use_system_audio)
{
	return handle->object->start(device_id, use_system_audio);
}

extern "C" __declspec(dllexport) void stopAudioZip(AudioZipHandle* handle)
{
	handle->object->stop();
}

extern "C" __declspec(dllexport) bool isMicMuted(AudioZipHandle * handle)
{
	return handle->object->isMicMuted();
}

extern "C" __declspec(dllexport) bool isSystemMuted(AudioZipHandle * handle)
{
	return handle->object->isSystemMuted();
}

extern "C" __declspec(dllexport) float getMicVolume(AudioZipHandle * handle)
{
	return handle->object->getMicVolume();
}

extern "C" __declspec(dllexport) float getSystemVolume(AudioZipHandle * handle)
{
	return handle->object->getSystemVolume();
}

extern "C" __declspec(dllexport) void setMicMute(AudioZipHandle * handle, bool value)
{
	handle->object->setMicMute(value);
}

extern "C" __declspec(dllexport) void setSystemMute(AudioZipHandle * handle, bool value)
{
	handle->object->setSystemMute(value);
}

extern "C" __declspec(dllexport) void setMicVolume(AudioZipHandle * handle, float volume)
{
	handle->object->setMicVolume(volume);
}

extern "C" __declspec(dllexport) void setSystemVolume(AudioZipHandle * handle, float volume)
{
	handle->object->setSystemVolume(volume);
}

extern "C" __declspec(dllexport) void releaseAudioZip(AudioZipHandle* handle)
{
	handle->object->setOnError(nullptr);
	handle->object->setOnEncode(nullptr);
	delete handle->object;
	delete handle;
}

extern "C" __declspec(dllexport) AudioUnZipHandle* createAudioUnZip(void* context, CALLBACK_ERROR on_error)
{
	return new AudioUnZipHandle(context, on_error);
}

extern "C" __declspec(dllexport) void playAudio(AudioUnZipHandle* handle, const void* data, int size)
{
	handle->object->play(data, size);
}

extern "C" __declspec(dllexport) void skipAudio(AudioUnZipHandle* handle, int count)
{
	handle->object->skip(count);
}

extern "C" __declspec(dllexport) int getDelayCount(AudioUnZipHandle* handle)
{
	return handle->object->getDelayCount();
}

extern "C" __declspec(dllexport) float getSpeakerVolume(AudioUnZipHandle* handle)
{
	return handle->object->getVolume();
}

extern "C" __declspec(dllexport) void setSpeakerVolume(AudioUnZipHandle* handle, float volume)
{
	handle->object->setVolume(volume);
}

extern "C" __declspec(dllexport) void releaseAudioUnZip(AudioUnZipHandle* handle)
{
	handle->object->setOnError(nullptr);
	delete handle->object;
	delete handle;
}