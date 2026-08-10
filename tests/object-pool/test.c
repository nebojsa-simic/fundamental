#include "fundamental/object-pool/object-pool.h"
#include "fundamental/console/console.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
}

// Helper to verify pointers are distinct
int all_distinct(void **ptrs, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		for (size_t j = i + 1; j < count; j++) {
			if (ptrs[i] == ptrs[j])
				return 0;
		}
	}
	return 1;
}

void test_pool_create_valid()
{
	ObjectPoolResult r = fun_object_pool_create(sizeof(uint64_t), 4);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	ObjectPool *pool = &r.value;

	if (!(pool->elementSize == sizeof(uint64_t))) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(pool->capacity == 4)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(pool->freeCount == 4)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(pool->freeList != (void *)0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	voidResult fr = fun_object_pool_destroy(pool);
	if (fr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("test_pool_create_valid");
}

void test_pool_create_too_small()
{
	ObjectPoolResult r = fun_object_pool_create(4, 8);
	if (r.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}
	if (!(r.value.memory == (void *)0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	print_test_result("test_pool_create_too_small");
}

void test_pool_create_zero_capacity()
{
	ObjectPoolResult r = fun_object_pool_create(32, 0);
	if (r.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}

	print_test_result("test_pool_create_zero_capacity");
}

void test_acquire_release_reuse()
{
#define POOL_SIZE 8
	ObjectPoolResult r = fun_object_pool_create(sizeof(uint64_t), POOL_SIZE);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	ObjectPool *pool = &r.value;

	void *ptrs[POOL_SIZE];

	// Acquire all slots
	for (size_t i = 0; i < POOL_SIZE; i++) {
		MemoryResult ar = fun_object_pool_acquire(pool);
		if (ar.error.code != 0) {
			fun_console_write_line("FAIL: ASSERT_NO_ERROR");
			return;
		}
		ptrs[i] = ar.value;
	}

	if (!(all_distinct(ptrs, POOL_SIZE))) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(pool->freeCount == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Release all
	for (size_t i = 0; i < POOL_SIZE; i++) {
		voidResult rr = fun_object_pool_release(pool, ptrs[i]);
		if (rr.error.code != 0) {
			fun_console_write_line("FAIL: ASSERT_NO_ERROR");
			return;
		}
	}

	if (!(pool->freeCount == POOL_SIZE)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Acquire again — should reuse same slots
	void *second[POOL_SIZE];
	for (size_t i = 0; i < POOL_SIZE; i++) {
		MemoryResult ar = fun_object_pool_acquire(pool);
		if (ar.error.code != 0) {
			fun_console_write_line("FAIL: ASSERT_NO_ERROR");
			return;
		}
		second[i] = ar.value;
	}

	if (!(all_distinct(second, POOL_SIZE))) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Release and destroy
	for (size_t i = 0; i < POOL_SIZE; i++) {
		fun_object_pool_release(pool, second[i]);
	}
	voidResult fr = fun_object_pool_destroy(pool);
	if (fr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

#undef POOL_SIZE
	print_test_result("test_acquire_release_reuse");
}

void test_acquire_exhausted()
{
	ObjectPoolResult r = fun_object_pool_create(sizeof(uint64_t), 2);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	ObjectPool *pool = &r.value;

	MemoryResult a1 = fun_object_pool_acquire(pool);
	if (a1.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	MemoryResult a2 = fun_object_pool_acquire(pool);
	if (a2.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	// Pool exhausted
	MemoryResult a3 = fun_object_pool_acquire(pool);
	if (a3.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}
	if (!(a3.value == (void *)0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	fun_object_pool_release(pool, a1.value);
	fun_object_pool_release(pool, a2.value);
	voidResult fr = fun_object_pool_destroy(pool);
	if (fr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("test_acquire_exhausted");
}

void test_release_invalid()
{
	ObjectPoolResult r = fun_object_pool_create(32, 4);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	ObjectPool *pool = &r.value;

	// Foreign slot (not from this pool)
	char foreign[32];
	voidResult rr = fun_object_pool_release(pool, foreign);
	if (rr.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}

	// NULL slot
	rr = fun_object_pool_release(pool, (void *)0);
	if (rr.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}

	voidResult fr = fun_object_pool_destroy(pool);
	if (fr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("test_release_invalid");
}

void test_destroy_with_leaks()
{
	ObjectPoolResult r = fun_object_pool_create(32, 4);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	ObjectPool *pool = &r.value;

	// Acquire 2, leave them outstanding
	fun_object_pool_acquire(pool);
	fun_object_pool_acquire(pool);

	voidResult dr = fun_object_pool_destroy(pool);
	if (dr.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}

	print_test_result("test_destroy_with_leaks");
}

void test_destroy_clean()
{
	ObjectPoolResult r = fun_object_pool_create(32, 4);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	ObjectPool *pool = &r.value;

	voidResult dr = fun_object_pool_destroy(pool);
	if (dr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("test_destroy_clean");
}

void test_query_functions()
{
	ObjectPoolResult r = fun_object_pool_create(24, 6);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	ObjectPool *pool = &r.value;

	if (!(fun_object_pool_free_count(pool) == 6)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_object_pool_capacity(pool) == 6)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_object_pool_element_size(pool) == 24)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	MemoryResult ar = fun_object_pool_acquire(pool);
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(fun_object_pool_free_count(pool) == 5)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_object_pool_capacity(pool) == 6)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	fun_object_pool_release(pool, ar.value);
	if (!(fun_object_pool_free_count(pool) == 6)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	voidResult fr = fun_object_pool_destroy(pool);
	if (fr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("test_query_functions");
}

typedef struct {
	uint64_t id;
	uint64_t value;
} TestSlot;

DEFINE_OBJECT_POOL_TYPE(TestSlot)

void test_typed_macro()
{
	ObjectPoolResult r = fun_object_pool_TestSlot_create(4);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	TestSlotPool pool;
	pool.pool = r.value;

	TestSlot *a = fun_object_pool_TestSlot_acquire(&pool);
	if (!(a != (TestSlot *)0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	a->id = 1;
	a->value = 42;

	TestSlot *b = fun_object_pool_TestSlot_acquire(&pool);
	if (!(b != (TestSlot *)0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	b->id = 2;
	b->value = 100;

	if (!(a != b)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(a->id == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(a->value == 42)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(b->id == 2)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(b->value == 100)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_object_pool_TestSlot_free_count(&pool) == 2)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	voidResult rr = fun_object_pool_TestSlot_release(&pool, a);
	if (rr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	rr = fun_object_pool_TestSlot_release(&pool, b);
	if (rr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	if (!(fun_object_pool_TestSlot_free_count(&pool) == 4)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	voidResult dr = fun_object_pool_TestSlot_destroy(&pool);
	if (dr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("test_typed_macro");
}

int main()
{
	fun_console_write_line("Running object pool tests:");
	test_pool_create_valid();
	test_pool_create_too_small();
	test_pool_create_zero_capacity();
	test_acquire_release_reuse();
	test_acquire_exhausted();
	test_release_invalid();
	test_destroy_with_leaks();
	test_destroy_clean();
	test_query_functions();
	test_typed_macro();
	fun_console_write_line("All tests passed!");
	return 0;
}
