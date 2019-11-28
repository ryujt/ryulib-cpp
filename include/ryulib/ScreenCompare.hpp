#pragma once

#include <ryulib/DeskZipUtils.hpp>

class ScreenCompare {
public:
	void prepare()
	{
		if (cells == nullptr) cells = new CellArray();
		screen_old_ = nullptr;
		screen_new_ = nullptr;
	}

	void execute(DeskScreen* screen)
	{
		screen_old_ = screen_new_;
		screen_new_ = screen;

		if (screen_old_ == nullptr) execute_first_screen();
		else execute_screen();

		if (OnCells_ != nullptr) OnCells_(this, screen_new_, cells);
	}

	void setOnCells(CellArrayEvent event) { OnCells_ = event; }

private:
	CellArray* cells = nullptr;
	DeskScreen* screen_old_ = nullptr;
	DeskScreen* screen_new_ = nullptr;

	CellArrayEvent OnCells_ = nullptr;

	void execute_first_screen()
	{
		cells->setSize((screen_new_->getBitmapWidth() / CELL_SIZE) * (screen_new_->getBitmapHeight() / CELL_SIZE));
		cells->setAll(true);
	}

	void execute_screen()
	{
		cells->setAll(false);

		char* pOld = (char*) screen_old_->getBuffer();
		char* pNew = (char*) screen_new_->getBuffer();

		for (int y = 0; y < screen_new_->getBitmapHeight(); y++) {
			for (int x = 0; x < (screen_new_->getBitmapWidth() / CELL_SIZE); x++) {
				int iy = y / CELL_SIZE;
				int i = x + (iy * (screen_new_->getBitmapWidth() / CELL_SIZE));

				if ((cells->get(i) == false) && (memcmp(pOld, pNew, LINE_SIZE) != 0)) {
					cells->set(i, true);
				}

				pOld = pOld + LINE_SIZE;
				pNew = pNew + LINE_SIZE;
			}
		}
	}
};