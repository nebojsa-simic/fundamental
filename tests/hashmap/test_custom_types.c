#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../include/hashmap/hashmap.h"

typedef struct {
	int x;
	int y;
} Point;

// Custom hash function for Point
static inline uint64_t fun_hash_Point(const void *key)
{
	const Point *p = (const Point *)key;
	// Combine x and y hashes
	return fun_hash_int(&p->x) ^ (fun_hash_int(&p->y) << 1);
}

// Custom equals function for Point
static inline bool fun_equals_Point(const void *k1, const void *k2)
{
	const Point *p1 = (const Point *)k1;
	const Point *p2 = (const Point *)k2;
	return p1->x == p2->x && p1->y == p2->y;
}

// Use custom macro with custom hash/equals
DEFINE_HASHMAP_TYPE_CUSTOM(Point, int, fun_hash_Point, fun_equals_Point)

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define ASSERT_ERROR_OK(result) assert((result).code == 0)
#define ASSERT_RESULT_OK(result) assert((result).error.code == 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

void test_point_hashmap_put_get(void)
{
	PointintHashMapResult create_result = fun_hashmap_Point_int_create(16);
	ASSERT_RESULT_OK(create_result);

	PointintHashMap map = create_result.value;

	Point p1 = { 10, 20 };
	Point p2 = { 30, 40 };
	Point p3 = { 50, 60 };

	ASSERT_ERROR_OK(fun_hashmap_Point_int_put(&map, p1, 100));
	ASSERT_ERROR_OK(fun_hashmap_Point_int_put(&map, p2, 200));
	ASSERT_ERROR_OK(fun_hashmap_Point_int_put(&map, p3, 300));

	assert(fun_hashmap_Point_int_get(&map, p1) == 100);
	assert(fun_hashmap_Point_int_get(&map, p2) == 200);
	assert(fun_hashmap_Point_int_get(&map, p3) == 300);

	ErrorResult destroy = fun_hashmap_Point_int_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("point_hashmap_put_get");
}

void test_point_hashmap_same_coordinates(void)
{
	PointintHashMapResult create_result = fun_hashmap_Point_int_create(16);
	ASSERT_RESULT_OK(create_result);

	PointintHashMap map = create_result.value;

	Point p1 = { 5, 5 };
	Point p2 = { 5, 5 }; // Same coordinates

	ASSERT_ERROR_OK(fun_hashmap_Point_int_put(&map, p1, 42));
	assert(fun_hashmap_Point_int_get(&map, p2) == 42);
	assert(fun_hashmap_Point_int_size(&map) == 1);

	ErrorResult destroy = fun_hashmap_Point_int_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("point_hashmap_same_coordinates");
}

void test_point_hashmap_update_value(void)
{
	PointintHashMapResult create_result = fun_hashmap_Point_int_create(16);
	ASSERT_RESULT_OK(create_result);

	PointintHashMap map = create_result.value;

	Point p = { 100, 200 };

	ASSERT_ERROR_OK(fun_hashmap_Point_int_put(&map, p, 1000));
	ASSERT_ERROR_OK(fun_hashmap_Point_int_put(&map, p, 9999));
	assert(fun_hashmap_Point_int_get(&map, p) == 9999);
	assert(fun_hashmap_Point_int_size(&map) == 1);

	ErrorResult destroy = fun_hashmap_Point_int_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("point_hashmap_update_value");
}

int main(void)
{
	printf("Running HashMap Custom Type Tests (Point struct)\n");
	printf("================================================\n\n");

	test_point_hashmap_put_get();
	test_point_hashmap_same_coordinates();
	test_point_hashmap_update_value();

	printf("\n================================================\n");
	printf("All custom type tests completed\n");
	return 0;
}
