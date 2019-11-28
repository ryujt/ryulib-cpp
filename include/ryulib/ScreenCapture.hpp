#pragma once

#include <ryulib/DeskZipUtils.hpp>
#include <ryulib/base.hpp>
#include <ryulib/debug_tools.hpp>
#include <boost/scope_exit.hpp>

class ScreenCapture {
public:
	ScreenCapture()
	{
	}

	~ScreenCapture()
	{
		//if (screen_old_ != nullptr) delete screen_old_;
		//if (screen_new_ != nullptr) delete screen_old_;
	}

	void prepare(int left, int top, int width, int height)
	{
		if (screen_old_ != nullptr) delete screen_old_;
		if (screen_new_ != nullptr) delete screen_old_;

		screen_old_ = new DeskScreen(left, top, width, height);
		screen_new_ = new DeskScreen(left, top, width, height);
	}

	void execute()
	{
		DeskScreen* temp = screen_old_;
		screen_old_ = screen_new_;
		screen_new_ = temp;
		do_capture(screen_new_);
	}

	void setOnScreen(ScreenEvent event) { OnScreen_ = event; }

private:
	DeskScreen* screen_old_ = nullptr;
	DeskScreen* screen_new_ = nullptr;

	void do_capture(DeskScreen* screen)
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

		hScrDC = GetDC(0);
		if (hScrDC == NULL) {
			DebugOutput::trace("GetDC() Error %d", GetLastError());
			return;
		}

		hMemDC = CreateCompatibleDC(hScrDC);
		if (hMemDC == NULL) {
			DebugOutput::trace("CreateCompatibleDC() Error %d", GetLastError());
			return;
		}

		hBit = CreateCompatibleBitmap(hScrDC, screen->getBitmapWidth(), screen->getBitmapHeight());
		if (hBit == NULL) {
			DebugOutput::trace("CreateCompatibleBitmap() Error %d", GetLastError());
			return;
		}

		hOldBitmap = (HBITMAP) SelectObject(hMemDC, hBit);

		if (BitBlt(hMemDC, 0, 0, screen->getBitmapWidth(), screen->getBitmapHeight(), hScrDC, 0, 0, SRCCOPY) == FALSE) {
			DebugOutput::trace("BitBlt() Error %d", GetLastError());
			return;
		}

		// 마우스 위치 얻기
		POINT curPoint;
		GetCursorPos(&curPoint);

		curPoint.x -= GetSystemMetrics(SM_XVIRTUALSCREEN);
		curPoint.y -= GetSystemMetrics(SM_YVIRTUALSCREEN);

		// hcurosr 얻어서 그리기
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
				DrawIconEx(hMemDC, curPoint.x, curPoint.y, cursorInfo.hCursor, 0, 0, 0, NULL, DI_NORMAL);
			}
		}

		bmpInfo.bmiHeader.biBitCount = 0;
		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.bmiHeader.biWidth = screen->getBitmapWidth();
		bmpInfo.bmiHeader.biHeight = -screen->getBitmapHeight();
		bmpInfo.bmiHeader.biPlanes = 1;
		bmpInfo.bmiHeader.biBitCount = PIXEL_SIZE * 8;
		bmpInfo.bmiHeader.biCompression = BI_RGB;
		bmpInfo.bmiHeader.biSizeImage = 0;
		bmpInfo.bmiHeader.biXPelsPerMeter = 0;
		bmpInfo.bmiHeader.biYPelsPerMeter = 0;
		bmpInfo.bmiHeader.biClrUsed = 0;
		bmpInfo.bmiHeader.biClrImportant = 0;

		// bitmap data 얻기
		GetDIBits(hMemDC, hBit, 0, screen->getBitmapHeight(), (PBYTE) screen->getBuffer(), &bmpInfo, DIB_RGB_COLORS);

		if (OnScreen_ != nullptr) OnScreen_(this, screen);
	}

	ScreenEvent OnScreen_ = nullptr;
};