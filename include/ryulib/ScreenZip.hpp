#pragma once

#include <ryulib/DeskZipUtils.hpp>
#include <zlib.h>
#include <turbojpeg.h>
#include <crc32c/crc32c.h>


class PixelEncoder {
public:
	~PixelEncoder()
	{
		//delete data_;
		//delete pixels_;
	}

	int execute(unsigned char* old_cell, unsigned char* new_cell)
	{
		__int32* pOld = (__int32*) old_cell;
		__int32* pNew = (__int32*) new_cell;

		state_ = STATE_BASE;
		current_ = pixels_;
		pixels_size_ = 0;

		for (int i = 0; i < (CELL_SIZE * CELL_SIZE); i++) {
			switch (state_) {
				case STATE_BASE: do_base(pOld, pNew); break;
				case STATE_SAME: do_same(pOld, pNew); break;
				case STATE_DIFF: do_diff(pOld, pNew); break;
			}

			if (pixels_size_ > PIXELS_SIZE_LIMIT) return -1;

			pOld++;
			pNew++;
		}

		do_finish();

		if (pixels_size_ > PIXELS_SIZE_LIMIT) return -1;

		size_ = BOX_SIZE * 2;
		if (compress2(data_, &size_, pixels_, pixels_size_, Z_BEST_COMPRESSION) != Z_OK) return -1;

		return 0;
	}

	unsigned char* getData() { return data_; }
	int getSize() { return size_; }

private:
	static const int PIXELS_SIZE_LIMIT = 1024 * 2;  // 픽셀 단위 압축 이후 크기가 PIXELS_SIZE_LIMIT를 넘으면 무시한다.

	static const int STATE_BASE = 0;
	static const int STATE_SAME = 1;
	static const int STATE_DIFF = 2;

	Bytef* data_ = (unsigned char*) malloc(BOX_SIZE * 2);  
	uLong size_ = 0;

	unsigned char* pixels_ = (unsigned char*) malloc(BOX_SIZE * 2);  
	unsigned long pixels_size_ = 0;

	int state_ = STATE_BASE;
	unsigned short count_ = 0;
	unsigned char* current_ = nullptr;  // 현재 데이터를 저장해야 할 곳
	unsigned char* offset_ = nullptr;  // 상태가 변경되어 작업이 시작된 위치 기록
	
	void do_base(__int32* old_pix, __int32* new_pix)
	{
		count_ = 1;
		offset_ = (unsigned char*) new_pix;

		if (*old_pix == *new_pix) state_ = STATE_SAME;
		else state_ = STATE_DIFF;
	}

	void do_same(__int32* old_pix, __int32* new_pix)
	{
		if (*old_pix == *new_pix) {
			count_++;
		} else {
			// 변화되지 않은 개수는 앞에 10000을 더해서 구별한다.
			count_ = count_ + 10000;

			memcpy(current_, &count_, sizeof(count_));
			current_ = current_ + sizeof(count_);
			pixels_size_ = pixels_size_ + sizeof(count_);

			count_ = 1;
			offset_ = (unsigned char*) new_pix;
			state_ = STATE_DIFF;
		}
	}

	void do_diff(__int32* old_pix, __int32* new_pix)
	{
		if (*old_pix == *new_pix) {
			memcpy(current_, &count_, sizeof(count_));
			current_ = current_ + sizeof(count_);
			pixels_size_ = pixels_size_ + sizeof(count_);

			// 변화된 픽셀 저장
			memcpy(current_, offset_, count_ * PIXEL_SIZE);
			current_ = current_ + (count_ * PIXEL_SIZE);
			pixels_size_ = pixels_size_ + (count_ * PIXEL_SIZE);

			count_ = 1;
			state_ = STATE_SAME;
		} else {
			count_++;
		}
	}

	void do_finish()
	{
		if (state_ == STATE_SAME) {
			// 변화되지 않은 개수는 앞에 10000을 더해서 구별한다.
			count_ = count_ + 10000;

			memcpy(current_, &count_, sizeof(count_));
			current_ = current_ + sizeof(count_);
			pixels_size_ = pixels_size_ + sizeof(count_);
		} else {
			memcpy(current_, &count_, sizeof(count_));
			current_ = current_ + sizeof(count_);
			pixels_size_ = pixels_size_ + sizeof(count_);

			// 변화된 픽셀 저장
			memcpy(current_, offset_, count_ * PIXEL_SIZE);
			current_ = current_ + (count_ * PIXEL_SIZE);
			pixels_size_ = pixels_size_ + (count_ * PIXEL_SIZE);
		}
	}
};


class JPegEncoder {
public:
	JPegEncoder()
	{
		jpeg_handle_ = tjInitCompress();
	}

	~JPegEncoder()
	{
		//if (jpeg_handle_ != nullptr) tjDestroy(jpeg_handle_);
	}

	int execute(unsigned char* src)
	{
		int result = tjCompress2(
			jpeg_handle_, src, CELL_SIZE, 0, CELL_SIZE, TJPF_BGRA, &header_, &jpeg_size_, 
			TJSAMP_422, 70, TJFLAG_FASTDCT + TJFLAG_BOTTOMUP);
		body_ = header_ + JPEG_HEADER_SIZE;
		return result;
	}

	unsigned char* getHeader() { return header_; }
	unsigned char* getBody() { return body_; }
	int getBodySize() { return jpeg_size_ - JPEG_HEADER_SIZE; }

private:
	tjhandle jpeg_handle_ = nullptr;

	unsigned char* header_ = nullptr;
	unsigned char* body_ = nullptr;  
	unsigned long jpeg_size_ = 0;
};


class ScreenZip {
public:
	void prepare()
	{
		screen_old_ = nullptr;
		screen_new_ = nullptr;
	}

	void execute(DeskScreen* screen, CellArray* cells)
	{
		screen_old_ = screen_new_;
		screen_new_ = screen;

		unsigned char new_cell[BOX_SIZE];
		unsigned char old_cell[BOX_SIZE];

		for (int i = 0; i < cells->size(); i++) {
			if (cells->get(i)) {
				screen->getDeskCell(i, new_cell);

				if (jpeg_encoder_.execute(new_cell) == 0) {
					DeskFrame* frame = new DeskFrame(ftJpeg, i, jpeg_encoder_.getBody(), jpeg_encoder_.getBodySize());
					frame->key = crc32c::Crc32c(jpeg_encoder_.getBody(), jpeg_encoder_.getBodySize());
					if (OnFrame_ != nullptr) OnFrame_(this, frame);
				}

				if (screen_old_ != nullptr) {
					screen_old_->getDeskCell(i, old_cell);
					if (pixel_encoder_.execute(old_cell, new_cell) == 0) {

						// JPEG보다 크기 작은 경우에만 PIXEL 압축 방식 적용
						if (pixel_encoder_.getSize() < jpeg_encoder_.getBodySize()) {
							DeskFrame* frame = new DeskFrame(ftPixel, i, pixel_encoder_.getData(), pixel_encoder_.getSize());
							if (OnFrame_ != nullptr) OnFrame_(this, frame);
						}
					}
				}
			}
		}
	}

	void setOnFrame(FrameEvent event) { OnFrame_ = event; }
private:
	DeskScreen* screen_old_ = nullptr;
	DeskScreen* screen_new_ = nullptr;

	PixelEncoder pixel_encoder_;
	JPegEncoder jpeg_encoder_;

	FrameEvent OnFrame_ = nullptr;
};
