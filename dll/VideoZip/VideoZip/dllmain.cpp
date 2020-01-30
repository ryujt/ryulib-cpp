// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <opencv2/opencv.hpp>
#include <ryulib/CameraList.hpp>
#include <ryulib/VideoZip.hpp>
#include <ryulib/VideoUnZip.hpp>

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
	bool open(int width, int height)
	{
		width_ = width;
		height_ = height;

		cap_.set(CAP_PROP_CONVERT_RGB, true);
		cap_.set(CAP_PROP_FRAME_WIDTH, width);
		cap_.set(CAP_PROP_FRAME_HEIGHT, height);
		cap_.open(0);
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
		Mat img_src;
		Mat img_dst;

		cap_.read(img_src);
		resize(img_src, img_dst, Size(width_, height_), 0, 0, INTER_LINEAR);
		video_zip_.encode(img_dst.data, 24);	
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

	int width_;
	int height_;
	int cam_width_;
	int cam_height_;
};

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

extern "C" __declspec(dllexport) bool openVideopZip(VideopZipHandle* handle, int width, int height)
{
	return handle->open(width, height);
}

extern "C" __declspec(dllexport) void closeVideopZip(VideopZipHandle* handle)
{
	handle->close();
}

extern "C" __declspec(dllexport) void encode(VideopZipHandle* handle)
{
	handle->encode();
}

extern "C" __declspec(dllexport) void* getData(VideopZipHandle* handle)
{
	return handle->getData();
}

extern "C" __declspec(dllexport) int getSize(VideopZipHandle* handle)
{
	return handle->getSize();
}

extern "C" __declspec(dllexport) VideoUnZipHandle* createVideoUnZip()
{
	return new VideoUnZipHandle();
}

extern "C" __declspec(dllexport) void releaseVideoUnZip(VideoUnZipHandle* handle)
{
	handle->close();
	delete handle;
}

extern "C" __declspec(dllexport) void openVideopUnZip(VideoUnZipHandle* handle, int width, int height)
{
	handle->open(width, height);
}

extern "C" __declspec(dllexport) void refresh(VideoUnZipHandle* handle)
{
	handle->refresh();
}

extern "C" __declspec(dllexport) void decode(VideoUnZipHandle* handle, void* data, int size)
{
	handle->decode(data, size);
}

extern "C" __declspec(dllexport) void* getBitmap(VideoUnZipHandle* handle)
{
	return handle->getBitmap();
}