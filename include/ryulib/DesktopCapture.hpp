#pragma once

#include <boost/scope_exit.hpp>
#include <ryulib/base.hpp>
#include <ryulib/Scheduler.hpp>
#include <ryulib/debug_tools.hpp>

const int BITMAP_PIXEL_SIZE = 4;
const int BITMAP_CELL_WIDTH = 8;
const int BITMAP_CELL_HEIGHT = 2;

typedef struct DesktopCaptureOption {
private:
	int target_handle = -1;
	int left = 0;
	int top = 0;
	int width = 0;
	int height = 0;
	int bitmap_width = 0;
	int bitmap_height = 0;
	int bitmap_size = 0;

public:
	int fps = 24;
	bool with_cursor = true;

	int getTargetHandle() { return target_handle; }

	int getLeft()
	{
		if (target_handle == -1) return left;

		tagRECT rect;
		GetWindowRect((HWND) target_handle, &rect);
		return rect.left;
	};

	int getTop()
	{
		if (target_handle == -1) return top;

		tagRECT rect;
		GetWindowRect((HWND) target_handle, &rect);
		return rect.top;
	};

	int getWidth() { return width; }
	int getHeight() { return height; }

	int getBitmapWidth() { return bitmap_width; }
	int getBitmapHeight() { return bitmap_height; }

	int getBitmapSize() { return bitmap_size; }

	void setTargetHandle(int value) 
	{ 
		target_handle = value; 

		if (target_handle == -1) return;

		tagRECT rect;
		GetWindowRect((HWND) target_handle, &rect);

		setLeft(rect.left);
		setTop(rect.top);
		setWidth(rect.right - rect.left);
		setHeight(rect.bottom - rect.top);
	}

	void setLeft(int value) { left = value; }
	void setTop(int value) { top = value; }

	void setWidth(int value)
	{
		width = value;
		bitmap_width = value;
		if ((bitmap_width % BITMAP_CELL_WIDTH) != 0) bitmap_width = bitmap_width + (BITMAP_CELL_WIDTH - (bitmap_width % BITMAP_CELL_WIDTH));
		bitmap_size = bitmap_width * bitmap_height * BITMAP_PIXEL_SIZE;
	}

	void setHeight(int value)
	{
		height = value;
		bitmap_height = value;
		if ((bitmap_height % BITMAP_CELL_HEIGHT) != 0) bitmap_height = bitmap_height + (BITMAP_CELL_HEIGHT - (bitmap_height % BITMAP_CELL_HEIGHT));
		bitmap_size = bitmap_width * bitmap_height * BITMAP_PIXEL_SIZE;
	}
};

class DesktopCapture {
public:
	DesktopCapture()
	{
		scheduler_.setOnRepeat([&]() {
			if (IsIconic((HWND)option_.getTargetHandle())) return;

			do_capture();
			if (option_.fps < 1) scheduler_.sleep(50);
			else scheduler_.sleep(1000 / option_.fps);
		});
	}

	~DesktopCapture()
	{
		if (bitmap_ != nullptr) free(bitmap_);
	}

	void start(DesktopCaptureOption option)
	{
		option_ = option;

		if (bitmap_ != nullptr) free(bitmap_);
		bitmap_ = malloc(option.getBitmapSize());

		scheduler_.start();
	}

	void stop()
	{
		scheduler_.stop();
	}

	void terminateAndWait()
	{
		scheduler_.terminateAndWait();
	}

	void setOnData(DataEvent event) { on_data_ = event; }

private:
	Scheduler scheduler_;
	DesktopCaptureOption option_;

	void* bitmap_ = nullptr;

	DataEvent on_data_ = nullptr;

	void do_capture()
	{
		BITMAPINFO bmpInfo;
		HDC hMemDC = NULL;
		HDC hScrDC = NULL;
		HBITMAP hBit = NULL;
		HBITMAP hOldBitmap = NULL;

		BOOST_SCOPE_EXIT(&hMemDC, &hScrDC, &hBit, &hOldBitmap)
		{
			if (hMemDC != NULL) {
				if (hOldBitmap != NULL) SelectObject(hMemDC, hOldBitmap);
				DeleteDC(hMemDC);
			}

			if (hBit != NULL) DeleteObject(hBit);
			if (hScrDC != NULL) DeleteDC(hScrDC);
		}
		BOOST_SCOPE_EXIT_END;

		if (option_.getTargetHandle() != -1) hScrDC = GetWindowDC((HWND)option_.getTargetHandle());
		else hScrDC = GetDC(0);
		if (hScrDC == NULL) {
			DebugOutput::trace("GetDC() Error %d", GetLastError());
			return;
		}

		hMemDC = CreateCompatibleDC(hScrDC);
		if (hMemDC == NULL) {
			DebugOutput::trace("CreateCompatibleDC() Error %d", GetLastError());
			return;
		}

		hBit = CreateCompatibleBitmap(hScrDC, option_.getBitmapWidth(), option_.getBitmapHeight());
		if (hBit == NULL) {
			DebugOutput::trace("CreateCompatibleBitmap() Error %d", GetLastError());
			return;
		}

		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBit);

		BOOL result;
		if (option_.getTargetHandle() != -1) {
			result = BitBlt(hMemDC, 0, 0, option_.getWidth(), option_.getHeight(), hScrDC, 0, 0, SRCCOPY);
		}
		else {
			result = BitBlt(hMemDC, 0, 0, option_.getWidth(), option_.getHeight(), hScrDC, option_.getLeft(), option_.getTop(), SRCCOPY);
		}
		if (result == FALSE) {
			DebugOutput::trace("BitBlt() Error %d", GetLastError());
			return;
		}

		if (option_.with_cursor) {
			POINT curPoint;
			GetCursorPos(&curPoint);

			curPoint.x -= GetSystemMetrics(SM_XVIRTUALSCREEN);
			curPoint.y -= GetSystemMetrics(SM_YVIRTUALSCREEN);

			{
				CURSORINFO cursorInfo;
				cursorInfo.cbSize = sizeof(CURSORINFO);

				if (GetCursorInfo(&cursorInfo)) {
					ICONINFO iconInfo;
					if (GetIconInfo(cursorInfo.hCursor, &iconInfo)) {
						curPoint.x = curPoint.x - iconInfo.xHotspot;
						curPoint.y = curPoint.y - iconInfo.yHotspot;
						if (iconInfo.hbmMask != NULL) DeleteObject(iconInfo.hbmMask);
						if (iconInfo.hbmColor != NULL) DeleteObject(iconInfo.hbmColor);
					}
					DrawIconEx(hMemDC, curPoint.x - option_.getLeft(), curPoint.y - option_.getTop(), cursorInfo.hCursor, 0, 0, 0, NULL, DI_NORMAL);
				}
			}
		}

		bmpInfo.bmiHeader.biBitCount = 0;
		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.bmiHeader.biWidth = option_.getBitmapWidth();
		bmpInfo.bmiHeader.biHeight = -option_.getBitmapHeight();
		bmpInfo.bmiHeader.biPlanes = 1;
		bmpInfo.bmiHeader.biBitCount = BITMAP_PIXEL_SIZE * 8;
		bmpInfo.bmiHeader.biCompression = BI_RGB;
		bmpInfo.bmiHeader.biSizeImage = 0;
		bmpInfo.bmiHeader.biXPelsPerMeter = 0;
		bmpInfo.bmiHeader.biYPelsPerMeter = 0;
		bmpInfo.bmiHeader.biClrUsed = 0;
		bmpInfo.bmiHeader.biClrImportant = 0;

		if (on_data_ != nullptr) {
			GetDIBits(hMemDC, hBit, 0, option_.getBitmapHeight(), (PBYTE) bitmap_, &bmpInfo, DIB_RGB_COLORS);
			on_data_(this, bitmap_, option_.getBitmapSize());
		}
	}
};
