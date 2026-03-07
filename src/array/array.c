#include "../../include/array/array.h"
#include "../../include/memory/memory.h"

// Implementation of the core functions that operate at the generic Array level
ArrayResult fun_array_create(size_t element_size, size_t initial_capacity)
{
	ArrayResult result = { .error = ERROR_RESULT_NO_ERROR };

	if (initial_capacity == 0) {
		initial_capacity = 1; // Minimum capacity to start
	}

	MemoryResult mem_result =
		fun_memory_allocate(initial_capacity * element_size);
	if (fun_error_is_error(mem_result.error)) {
		result.error = mem_result.error;
		return result;
	}

	// Set up the array structure
	result.value.data = mem_result.value;
	result.value.count = 0;
	result.value.capacity = initial_capacity;
	result.value.element_size = element_size;

	return result;
}

ErrorResult fun_array_push_common(Array *array, void *element)
{
	if (!array || !element) {
		return ERROR_RESULT_NULL_POINTER;
	}

	// Check if we need to grow the array
	if (array->count >= array->capacity) {
		// Calculate new capacity (double the size for amortized efficiency)
		size_t new_capacity = array->capacity * 2 ? array->capacity : 1;

		MemoryResult new_block_result = fun_memory_reallocate(
			array->data, new_capacity * array->element_size);
		if (fun_error_is_error(new_block_result.error)) {
			return fun_error_result(ERROR_CODE_REALLOCATION_FAILED,
									"Could not grow array");
		}

		array->data = new_block_result.value;
		array->capacity = new_capacity;
	}

	// Copy the element to the end of the array
	size_t pos = array->count;
	void *destptr = (char *)(array->data) + pos * array->element_size;
	// Simple byte copy since we know the element size
	for (size_t i = 0; i < array->element_size; i++) {
		((char *)destptr)[i] = ((char *)element)[i];
	}

	// Increase the count
	array->count++;
	return ERROR_RESULT_NO_ERROR;
}

ErrorResult fun_array_get_common(Array *array, size_t index, void *out_element)
{
	if (!array || !out_element) {
		return ERROR_RESULT_NULL_POINTER;
	}

	if (index >= array->count) {
		return fun_error_result(ERROR_CODE_INVALID_INDEX,
								"Index out of bounds for array access");
	}

	// Copy the element from the array to output
	void *srceptr = (char *)(array->data) + index * array->element_size;
	// Simple byte copy
	for (size_t i = 0; i < array->element_size; i++) {
		((char *)out_element)[i] = ((char *)srceptr)[i];
	}

	return ERROR_RESULT_NO_ERROR;
}

ErrorResult fun_array_set_common(Array *array, size_t index, void *element)
{
	if (!array || !element) {
		return ERROR_RESULT_NULL_POINTER;
	}

	if (index >= array->count) {
		return fun_error_result(ERROR_CODE_INVALID_INDEX,
								"Index out of bounds for array set");
	}

	// Copy the element to the array data
	void *destptr = (char *)(array->data) + index * array->element_size;
	// Simple byte copy
	for (size_t i = 0; i < array->element_size; i++) {
		((char *)destptr)[i] = ((char *)element)[i];
	}

	return ERROR_RESULT_NO_ERROR;
}

size_t fun_array_size_common(Array *array)
{
	if (!array)
		return 0;
	return array->count;
}

size_t fun_array_capacity_common(Array *array)
{
	if (!array)
		return 0;
	return array->capacity;
}

ErrorResult fun_array_destroy_common(Array *array)
{
	if (!array) {
		return ERROR_RESULT_NULL_POINTER;
	}

	if (array->data) {
		voidResult free_res = fun_memory_free((Memory *)&array->data);
		array->data = NULL;
		array->count = 0;
		array->capacity = 0;
		array->element_size = 0;
		return free_res.error;
	}

	return ERROR_RESULT_NO_ERROR;
}

// Type-specific implementations for int

IntArrayResult fun_array_int_create(size_t initial_capacity)
{
	IntArrayResult result = { .error = ERROR_RESULT_NO_ERROR };

	ArrayResult array_result = fun_array_create(sizeof(int), initial_capacity);
	if (fun_error_is_error(array_result.error)) {
		result.error = array_result.error;
		return result;
	}

	result.value.array = array_result.value;
	return result;
}

ErrorResult fun_array_int_push(IntArray *array, int value)
{
	return fun_array_push_common(&array->array, &value);
}

int fun_array_int_get(IntArray *array, size_t index)
{
	int value = 0;
	ErrorResult result = fun_array_get_common(&array->array, index, &value);
	if (fun_error_is_ok(result)) {
		return value;
	} else {
		// Return some indication of error (0 is default for error case)
		return 0;
	}
}

size_t fun_array_int_size(IntArray *array)
{
	return fun_array_size_common(&array->array);
}

ErrorResult fun_array_int_destroy(IntArray *array)
{
	return fun_array_destroy_common(&array->array);
}

// Type-specific implementations for char

CharArrayResult fun_array_char_create(size_t initial_capacity)
{
	CharArrayResult result = { .error = ERROR_RESULT_NO_ERROR };

	ArrayResult array_result = fun_array_create(sizeof(char), initial_capacity);
	if (fun_error_is_error(array_result.error)) {
		result.error = array_result.error;
		return result;
	}

	result.value.array = array_result.value;
	return result;
}

ErrorResult fun_array_char_push(CharArray *array, char value)
{
	return fun_array_push_common(&array->array, &value);
}

char fun_array_char_get(CharArray *array, size_t index)
{
	char value = 0;
	ErrorResult result = fun_array_get_common(&array->array, index, &value);
	if (fun_error_is_ok(result)) {
		return value;
	} else {
		return 0; // Default for error case
	}
}

size_t fun_array_char_size(CharArray *array)
{
	return fun_array_size_common(&array->array);
}

ErrorResult fun_array_char_destroy(CharArray *array)
{
	return fun_array_destroy_common(&array->array);
}

// Type-specific implementations for double

DoubleArrayResult fun_array_double_create(size_t initial_capacity)
{
	DoubleArrayResult result = { .error = ERROR_RESULT_NO_ERROR };

	ArrayResult array_result =
		fun_array_create(sizeof(double), initial_capacity);
	if (fun_error_is_error(array_result.error)) {
		result.error = array_result.error;
		return result;
	}

	result.value.array = array_result.value;
	return result;
}

ErrorResult fun_array_double_push(DoubleArray *array, double value)
{
	return fun_array_push_common(&array->array, &value);
}

double fun_array_double_get(DoubleArray *array, size_t index)
{
	double value = 0.0;
	ErrorResult result = fun_array_get_common(&array->array, index, &value);
	if (fun_error_is_ok(result)) {
		return value;
	} else {
		return 0.0; // Default for error case
	}
}

size_t fun_array_double_size(DoubleArray *array)
{
	return fun_array_size_common(&array->array);
}

ErrorResult fun_array_double_destroy(DoubleArray *array)
{
	return fun_array_destroy_common(&array->array);
}

// Type-specific implementations for pointer (void*)

PointerArrayResult fun_array_pointer_create(size_t initial_capacity)
{
	PointerArrayResult result = { .error = ERROR_RESULT_NO_ERROR };

	ArrayResult array_result =
		fun_array_create(sizeof(void *), initial_capacity);
	if (fun_error_is_error(array_result.error)) {
		result.error = array_result.error;
		return result;
	}

	result.value.array = array_result.value;
	return result;
}

ErrorResult fun_array_pointer_push(PointerArray *array, void *value)
{
	return fun_array_push_common(&array->array, &value);
}

void *fun_array_pointer_get(PointerArray *array, size_t index)
{
	void *value = NULL;
	ErrorResult result = fun_array_get_common(&array->array, index, &value);
	if (fun_error_is_ok(result)) {
		return value;
	} else {
		return NULL; // Default for error case
	}
}

size_t fun_array_pointer_size(PointerArray *array)
{
	return fun_array_size_common(&array->array);
}

ErrorResult fun_array_pointer_destroy(PointerArray *array)
{
	return fun_array_destroy_common(&array->array);
}