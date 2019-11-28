// GetMonitorInfo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <ryulib\graphics.hpp>


int _tmain(int argc, _TCHAR* argv[])
{
	int monitor_count = get_monitor_count();

	printf("monitor_count: %d \n\n", monitor_count);

	for (int i = 0; i < monitor_count; i++) {
		printf("monitor: %d \n", i);
		printf("get_monitor_width: %d \n", get_monitor_width(i));
		printf("get_monitor_height: %d \n\n", get_monitor_height(i));
	}
	
	return 0;
}

