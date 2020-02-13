// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <ryulib/VideoUnZip.hpp>


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

class VideoUnZipHandle {
public:
	void open(int width, int height)
	{
		width_ = width;
		height_ = height;

		video_unzip.open(width, height);
	}

	void close()
	{
		video_unzip.close();
	}

	void refresh()
	{
		video_unzip.refresh();
	}

	void decode(void* data, int size)
	{
		video_unzip.decode(data, size);
	}

	void* getBitmap()
	{
		return video_unzip.getBitmap(); 
	}

private:
	VideoUnZip video_unzip;

	int width_;
	int height_;
};

extern "C" __declspec(dllexport) VideoUnZipHandle* createVideoUnZip()
{
	return new VideoUnZipHandle();
}

extern "C" __declspec(dllexport) void releaseVideoUnZip(VideoUnZipHandle* handle)
{
	handle->close();
	delete handle;
}

extern "C" __declspec(dllexport) void openVideoUnZip(VideoUnZipHandle* handle, int width, int height)
{
	handle->open(width, height);
}

extern "C" __declspec(dllexport) void refreshVideoUnZip(VideoUnZipHandle* handle)
{
	handle->refresh();
}

extern "C" __declspec(dllexport) void decodeVideoUnZip(VideoUnZipHandle* handle, void* data, int size)
{
	handle->decode(data, size);
}

extern "C" __declspec(dllexport) void* getVideoUnZipBitmap(VideoUnZipHandle* handle)
{
	return handle->getBitmap();
}