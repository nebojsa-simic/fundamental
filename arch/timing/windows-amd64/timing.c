#include <stdint.h>
#include <windows.h>

int64_t fun_timing_now_unix(void)
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	uint64_t ticks = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
	const uint64_t epoch_diff =
		116444736000000000ULL; /* 1601-01-01 -> 1970-01-01, 100ns */
	return (int64_t)((ticks - epoch_diff) / 10000000ULL);
}

uint64_t fun_timing_now_ns(void)
{
	static LARGE_INTEGER freq = { .QuadPart = 0 };
	if (freq.QuadPart == 0)
		QueryPerformanceFrequency(&freq);
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	uint64_t secs = (uint64_t)counter.QuadPart / (uint64_t)freq.QuadPart;
	uint64_t rem = (uint64_t)counter.QuadPart % (uint64_t)freq.QuadPart;
	return secs * 1000000000ULL + rem * 1000000000ULL / (uint64_t)freq.QuadPart;
}
