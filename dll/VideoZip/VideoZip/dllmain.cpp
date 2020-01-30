// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <ryulib/CameraList.hpp>

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

extern "C" __declspec(dllexport) int getCameraCount()
{
	CameraList cameralist;
	cameralist.update();
	return cameralist.size();
}

extern "C" __declspec(dllexport) char* getCameraName(int index)
{
	static char name[256];
	CameraList cameralist;
	cameralist.update();
	strcpy_s(name, cameralist.getName(index).c_str());
	return name;
}