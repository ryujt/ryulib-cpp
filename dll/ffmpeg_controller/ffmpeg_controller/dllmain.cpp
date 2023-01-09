// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "FFmpegController.hpp"
#include <Windows.h>
#include <string>

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

extern "C" __declspec(dllexport) FFmpegController* create_ffmpeg_controller()
{
	DebugOutput::trace("create_ffmpeg_controller");
	return new FFmpegController();
}

extern "C" __declspec(dllexport) void destory_ffmpeg_controller(FFmpegController* handle)
{
	DebugOutput::trace("destory_ffmpeg_controller");
	delete handle;
}

extern "C" __declspec(dllexport) bool open_ffmpeg_controller(FFmpegController* handle, const char* json)
{
	return handle->open(json);
}

extern "C" __declspec(dllexport) void close_ffmpeg_controller(FFmpegController* handle)
{
	handle->close();
}