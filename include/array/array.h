#ifndef LIBRARY_ARRAY_H
#define LIBRARY_ARRAY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "../error/error.h"
#include "../memory/memory.h"

// Forward declaration of the array structure
typedef struct {
	void *data; // Dynamically allocated memory block
	size_t count; // Number of elements currently in array
	size_t capacity; // Maximum elements before resize
	size_t element_size; // Size of each element in bytes
} Array;

// Result type for array operations
typedef struct {
	Array value;
	ErrorResult error;
} ArrayResult;

// Define error codes specific to arrays
#define ERROR_CODE_INVALID_INDEX 101
#define ERROR_CODE_ARRAY_FULL 102
#define ERROR_CODE_REALLOCATION_FAILED 103

// Function pointers for element-specific operations needed by generic array
typedef void (*ElementCopyFn)(void *dest, const void *src, size_t element_size);
typedef void (*ElementDestroyFn)(void *element);

// Core Array API for generic operations
ArrayResult fun_array_create(size_t element_size, size_t initial_capacity);
ErrorResult fun_array_push_common(Array *array, void *element);
ErrorResult fun_array_get_common(Array *array, size_t index, void *out_element);
ErrorResult fun_array_set_common(Array *array, size_t index, void *element);
size_t fun_array_size_common(Array *array);
size_t fun_array_capacity_common(Array *array);
ErrorResult fun_array_destroy_common(Array *array);

// Type-specific Array API - manually defined for each type
// Int Array
typedef struct {
	Array array;
} IntArray;

typedef struct {
	IntArray value;
	ErrorResult error;
} IntArrayResult;

IntArrayResult fun_array_int_create(size_t initial_capacity);
ErrorResult fun_array_int_push(IntArray *array, int value);
int fun_array_int_get(IntArray *array, size_t index);
size_t fun_array_int_size(IntArray *array);
ErrorResult fun_array_int_destroy(IntArray *array);

// Char Array
typedef struct {
	Array array;
} CharArray;

typedef struct {
	CharArray value;
	ErrorResult error;
} CharArrayResult;

CharArrayResult fun_array_char_create(size_t initial_capacity);
ErrorResult fun_array_char_push(CharArray *array, char value);
char fun_array_char_get(CharArray *array, size_t index);
size_t fun_array_char_size(CharArray *array);
ErrorResult fun_array_char_destroy(CharArray *array);

// Double Array
typedef struct {
	Array array;
} DoubleArray;

typedef struct {
	DoubleArray value;
	ErrorResult error;
} DoubleArrayResult;

DoubleArrayResult fun_array_double_create(size_t initial_capacity);
ErrorResult fun_array_double_push(DoubleArray *array, double value);
double fun_array_double_get(DoubleArray *array, size_t index);
size_t fun_array_double_size(DoubleArray *array);
ErrorResult fun_array_double_destroy(DoubleArray *array);

// Pointer Array
typedef struct {
	Array array;
} PointerArray;

typedef struct {
	PointerArray value;
	ErrorResult error;
} PointerArrayResult;

PointerArrayResult fun_array_pointer_create(size_t initial_capacity);
ErrorResult fun_array_pointer_push(PointerArray *array, void *value);
void *fun_array_pointer_get(PointerArray *array, size_t index);
size_t fun_array_pointer_size(PointerArray *array);
ErrorResult fun_array_pointer_destroy(PointerArray *array);

// Additional utility functions
static inline bool fun_array_is_empty(const Array *array)
{
	return array && array->count == 0;
}

static inline bool fun_array_is_full(const Array *array)
{
	return array && array->count >= array->capacity;
}

static inline bool fun_array_int_is_empty(const IntArray *array)
{
	return array && array->array.count == 0;
}

static inline bool fun_array_char_is_empty(const CharArray *array)
{
	return array && array->array.count == 0;
}

static inline bool fun_array_double_is_empty(const DoubleArray *array)
{
	return array && array->array.count == 0;
}

static inline bool fun_array_pointer_is_empty(const PointerArray *array)
{
	return array && array->array.count == 0;
}

#endif // LIBRARY_ARRAY_H