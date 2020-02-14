// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <ryulib/debug_tools.hpp>
#include <opencv2/opencv.hpp>

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

using namespace cv;

class ImageResize {
public:
    void readBitmap32(void* bitmap, int width, int height)
    {
        try {
            img_src_ = Mat(height, width, CV_8UC4, bitmap);
        } catch (...) {
            DebugOutput::trace("readBitmap32 Error - width: %d, height: %d", width, height);
        }
    }

    void* resizeBitmap32(int width, int height)
    {
        try {
            resize(img_src_, img_dst_, Size(width, height), 0, 0, INTER_LINEAR);
        } catch (...) {
            DebugOutput::trace("resizeBitmap32 Error - width: %d, height: %d", width, height);
            return NULL;
        }
        return img_dst_.data;
    }

private:
    Mat img_src_;
    Mat img_dst_;
};

extern "C" __declspec(dllexport) ImageResize* createImageResize()
{
    return new ImageResize();
}

extern "C" __declspec(dllexport) void releaseImageResize(ImageResize* handle)
{
    delete handle;
}

extern "C" __declspec(dllexport) void readBitmap32(ImageResize* obj, void* bitmap, int width, int height)
{
    obj->readBitmap32(bitmap, width, height);
}

extern "C" __declspec(dllexport) void* resizeBitmap32(ImageResize* obj, int width, int height)
{
    return obj->resizeBitmap32(width, height);
}