// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <opencv2/opencv.hpp>
#include <ryulib/CameraList.hpp>
#include <ryulib/VideoZip.hpp>

using namespace cv;


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

class VideopZipHandle {
public:
	bool open(int index, int width, int height)
	{
		width_ = width;
		height_ = height;

		cap_.set(CAP_PROP_CONVERT_RGB, true);
		cap_.set(CAP_PROP_FRAME_WIDTH, width);
		cap_.set(CAP_PROP_FRAME_HEIGHT, height);
		cap_.open(index);
		if (cap_.isOpened() == false) return false;

		cam_width_  = cap_.get(CAP_PROP_FRAME_WIDTH);
		cam_height_ = cap_.get(CAP_PROP_FRAME_HEIGHT);

		video_zip_.open(width, height);

		return true;
	}

	void close()
	{
		video_zip_.close();
	}

	void encode()
	{
		cap_.read(img_src_);
		resize(img_src_, img_dst_, Size(width_, height_), 0, 0, INTER_LINEAR);
		video_zip_.encode(img_dst_.data, 24);	
	}

	void* getBitmap()
	{
		return img_dst_.data; 
	}

	void* getData() 
	{ 
		return video_zip_.getData(); 
	}

	int getSize()
	{ 
		return video_zip_.getSize(); 
	}

private:
	VideoCapture cap_;
	VideoZip video_zip_;

	Mat img_src_;
	Mat img_dst_;

	int width_;
	int height_;
	int cam_width_;
	int cam_height_;
};


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

extern "C" __declspec(dllexport) VideopZipHandle* createVideoZip()
{
	return new VideopZipHandle();
}

extern "C" __declspec(dllexport) void releaseVideoZip(VideopZipHandle* handle)
{
	handle->close();
	delete handle;
}

extern "C" __declspec(dllexport) bool openVideoZip(VideopZipHandle* handle, int index, int width, int height)
{
	return handle->open(index, width, height);
}

extern "C" __declspec(dllexport) void closeVideoZip(VideopZipHandle* handle)
{
	handle->close();
}

extern "C" __declspec(dllexport) void encodeVideoZip(VideopZipHandle* handle)
{
	handle->encode();
}

extern "C" __declspec(dllexport) void* getVideoZipBitmap(VideopZipHandle* handle)
{
	return handle->getBitmap();
}

extern "C" __declspec(dllexport) void* getVideoZipData(VideopZipHandle* handle)
{
	return handle->getData();
}

extern "C" __declspec(dllexport) int getVideoZipSize(VideopZipHandle* handle)
{
	return handle->getSize();
}