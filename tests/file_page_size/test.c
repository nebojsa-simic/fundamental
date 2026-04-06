#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "../../../arch/file/linux-amd64/page_size.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

void test_get_page_size_returns_positive(void)
{
	uint64_t page_size = get_page_size();
	assert(page_size > 0);
	assert(page_size % 2 == 0); // Must be power of 2
	print_test_result("get_page_size_returns_positive");
}

void test_get_page_size_matches_sysconf(void)
{
	uint64_t cached = get_page_size();
	long sysconf_size = sysconf(_SC_PAGESIZE);

	assert(sysconf_size > 0);
	assert(cached == (uint64_t)sysconf_size);
	print_test_result("get_page_size_matches_sysconf");
}

void test_get_page_size_cached(void)
{
	// First call - will cache
	uint64_t first = get_page_size();

	// Second call - should return cached value
	uint64_t second = get_page_size();

	assert(first == second);
	assert(first > 0);
	print_test_result("get_page_size_cached");
}

void test_get_page_size_power_of_two(void)
{
	uint64_t page_size = get_page_size();

	// Check if power of 2: (n & (n-1)) == 0
	assert((page_size & (page_size - 1)) == 0);
	print_test_result("get_page_size_power_of_two");
}

void test_get_page_size_reasonable(void)
{
	uint64_t page_size = get_page_size();

	// Page size should be at least 4KB and at most 64KB for common architectures
	assert(page_size >= 4096);
	assert(page_size <= 65536);
	print_test_result("get_page_size_reasonable");
}

void test_page_size_alignment(void)
{
	uint64_t page_size = get_page_size();

	// Test various offsets
	uint64_t offsets[] = { 0, 100, 4095, 4096, 5000, 10000 };
	size_t num_offsets = sizeof(offsets) / sizeof(offsets[0]);

	for (size_t i = 0; i < num_offsets; i++) {
		uint64_t offset = offsets[i];
		uint64_t aligned = offset & ~(page_size - 1);

		// Aligned offset should be <= original
		assert(aligned <= offset);

		// Aligned offset should be page-aligned
		assert((aligned % page_size) == 0);

		// Next page should be > offset
		uint64_t next_page = aligned + page_size;
		assert(next_page > offset);
	}
	print_test_result("page_size_alignment");
}

int main(void)
{
	printf("Running file page size tests:\n");

	test_get_page_size_returns_positive();
	test_get_page_size_matches_sysconf();
	test_get_page_size_cached();
	test_get_page_size_power_of_two();
	test_get_page_size_reasonable();
	test_page_size_alignment();

	printf("All file page size tests passed!\n");
	return 0;
}
