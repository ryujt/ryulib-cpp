#pragma once

#include <ryulib/DeskScreen.hpp>
#include <ryulib/DeskZipUtils.hpp>
#include <ryulib/base.hpp>

class ScreenSplit {
public:
	void start(const DeskScreen* screen)
	{

	}

	void execute(const DeskScreen* screen)
	{

	}

	void setOnCell(DataEvent event) { OnCell_ = event;  }
private:
	DataEvent OnCell_ = nullptr;
};