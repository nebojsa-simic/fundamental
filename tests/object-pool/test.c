#include <assert.h>
#include <stdio.h>
#include "fundamental/object-pool/object-pool.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
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
	ASSERT_NO_ERROR(r);
	ObjectPool *pool = &r.value;

	assert(pool->elementSize == sizeof(uint64_t));
	assert(pool->capacity == 4);
	assert(pool->freeCount == 4);
	assert(pool->freeList != (void *)0);

	voidResult fr = fun_object_pool_destroy(pool);
	ASSERT_NO_ERROR(fr);

	print_test_result("test_pool_create_valid");
}

void test_pool_create_too_small()
{
	ObjectPoolResult r = fun_object_pool_create(4, 8);
	ASSERT_ERROR(r);
	assert(r.value.memory == (void *)0);

	print_test_result("test_pool_create_too_small");
}

void test_pool_create_zero_capacity()
{
	ObjectPoolResult r = fun_object_pool_create(32, 0);
	ASSERT_ERROR(r);

	print_test_result("test_pool_create_zero_capacity");
}

void test_acquire_release_reuse()
{
#define POOL_SIZE 8
	ObjectPoolResult r = fun_object_pool_create(sizeof(uint64_t), POOL_SIZE);
	ASSERT_NO_ERROR(r);
	ObjectPool *pool = &r.value;

	void *ptrs[POOL_SIZE];

	// Acquire all slots
	for (size_t i = 0; i < POOL_SIZE; i++) {
		MemoryResult ar = fun_object_pool_acquire(pool);
		ASSERT_NO_ERROR(ar);
		ptrs[i] = ar.value;
	}

	assert(all_distinct(ptrs, POOL_SIZE));
	assert(pool->freeCount == 0);

	// Release all
	for (size_t i = 0; i < POOL_SIZE; i++) {
		voidResult rr = fun_object_pool_release(pool, ptrs[i]);
		ASSERT_NO_ERROR(rr);
	}

	assert(pool->freeCount == POOL_SIZE);

	// Acquire again — should reuse same slots
	void *second[POOL_SIZE];
	for (size_t i = 0; i < POOL_SIZE; i++) {
		MemoryResult ar = fun_object_pool_acquire(pool);
		ASSERT_NO_ERROR(ar);
		second[i] = ar.value;
	}

	assert(all_distinct(second, POOL_SIZE));

	// Release and destroy
	for (size_t i = 0; i < POOL_SIZE; i++) {
		fun_object_pool_release(pool, second[i]);
	}
	voidResult fr = fun_object_pool_destroy(pool);
	ASSERT_NO_ERROR(fr);

#undef POOL_SIZE
	print_test_result("test_acquire_release_reuse");
}

void test_acquire_exhausted()
{
	ObjectPoolResult r = fun_object_pool_create(sizeof(uint64_t), 2);
	ASSERT_NO_ERROR(r);
	ObjectPool *pool = &r.value;

	MemoryResult a1 = fun_object_pool_acquire(pool);
	ASSERT_NO_ERROR(a1);
	MemoryResult a2 = fun_object_pool_acquire(pool);
	ASSERT_NO_ERROR(a2);

	// Pool exhausted
	MemoryResult a3 = fun_object_pool_acquire(pool);
	ASSERT_ERROR(a3);
	assert(a3.value == (void *)0);

	fun_object_pool_release(pool, a1.value);
	fun_object_pool_release(pool, a2.value);
	voidResult fr = fun_object_pool_destroy(pool);
	ASSERT_NO_ERROR(fr);

	print_test_result("test_acquire_exhausted");
}

void test_release_invalid()
{
	ObjectPoolResult r = fun_object_pool_create(32, 4);
	ASSERT_NO_ERROR(r);
	ObjectPool *pool = &r.value;

	// Foreign slot (not from this pool)
	char foreign[32];
	voidResult rr = fun_object_pool_release(pool, foreign);
	ASSERT_ERROR(rr);

	// NULL slot
	rr = fun_object_pool_release(pool, (void *)0);
	ASSERT_ERROR(rr);

	voidResult fr = fun_object_pool_destroy(pool);
	ASSERT_NO_ERROR(fr);

	print_test_result("test_release_invalid");
}

void test_destroy_with_leaks()
{
	ObjectPoolResult r = fun_object_pool_create(32, 4);
	ASSERT_NO_ERROR(r);
	ObjectPool *pool = &r.value;

	// Acquire 2, leave them outstanding
	fun_object_pool_acquire(pool);
	fun_object_pool_acquire(pool);

	voidResult dr = fun_object_pool_destroy(pool);
	ASSERT_ERROR(dr);

	print_test_result("test_destroy_with_leaks");
}

void test_destroy_clean()
{
	ObjectPoolResult r = fun_object_pool_create(32, 4);
	ASSERT_NO_ERROR(r);
	ObjectPool *pool = &r.value;

	voidResult dr = fun_object_pool_destroy(pool);
	ASSERT_NO_ERROR(dr);

	print_test_result("test_destroy_clean");
}

void test_query_functions()
{
	ObjectPoolResult r = fun_object_pool_create(24, 6);
	ASSERT_NO_ERROR(r);
	ObjectPool *pool = &r.value;

	assert(fun_object_pool_free_count(pool) == 6);
	assert(fun_object_pool_capacity(pool) == 6);
	assert(fun_object_pool_element_size(pool) == 24);

	MemoryResult ar = fun_object_pool_acquire(pool);
	ASSERT_NO_ERROR(ar);
	assert(fun_object_pool_free_count(pool) == 5);
	assert(fun_object_pool_capacity(pool) == 6);

	fun_object_pool_release(pool, ar.value);
	assert(fun_object_pool_free_count(pool) == 6);

	voidResult fr = fun_object_pool_destroy(pool);
	ASSERT_NO_ERROR(fr);

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
	ASSERT_NO_ERROR(r);
	TestSlotPool pool;
	pool.pool = r.value;

	TestSlot *a = fun_object_pool_TestSlot_acquire(&pool);
	assert(a != (TestSlot *)0);
	a->id = 1;
	a->value = 42;

	TestSlot *b = fun_object_pool_TestSlot_acquire(&pool);
	assert(b != (TestSlot *)0);
	b->id = 2;
	b->value = 100;

	assert(a != b);
	assert(a->id == 1);
	assert(a->value == 42);
	assert(b->id == 2);
	assert(b->value == 100);
	assert(fun_object_pool_TestSlot_free_count(&pool) == 2);

	voidResult rr = fun_object_pool_TestSlot_release(&pool, a);
	ASSERT_NO_ERROR(rr);
	rr = fun_object_pool_TestSlot_release(&pool, b);
	ASSERT_NO_ERROR(rr);

	assert(fun_object_pool_TestSlot_free_count(&pool) == 4);

	voidResult dr = fun_object_pool_TestSlot_destroy(&pool);
	ASSERT_NO_ERROR(dr);

	print_test_result("test_typed_macro");
}

int main()
{
	printf("Running object pool tests:\n");
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
	printf("All tests passed!\n");
	return 0;
}
