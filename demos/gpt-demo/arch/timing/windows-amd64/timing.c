#include "../../../timing.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

double demo_time_now(void)
{
	LARGE_INTEGER freq, counter;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&counter);
	return (double)counter.QuadPart / (double)freq.QuadPart;
}
