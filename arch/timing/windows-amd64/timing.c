#include <stdint.h>
#include <windows.h>

uint64_t fun_timing_now_ns(void)
{
	static LARGE_INTEGER freq = { .QuadPart = 0 };
	if (freq.QuadPart == 0)
		QueryPerformanceFrequency(&freq);
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (uint64_t)(counter.QuadPart * 1000000000ULL / freq.QuadPart);
}
