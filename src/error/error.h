#ifndef LIBRARY_ERROR_RESULT_H
#define LIBRARY_ERROR_RESULT_H

#include <stdint.h>
#include <stddef.h>

#define ERROR_CODE_NO_ERROR 0;

typedef struct {
    uint8_t Code;
    const char* Message;
} ErrorResult;

#define DEFINE_RESULT_TYPE(ResultType) \
typedef struct { \
    ResultType Value; \
    ErrorResult Error; \
} ResultType##Result

// Define result types for common simple types
DEFINE_RESULT_TYPE(char);
DEFINE_RESULT_TYPE(float);
DEFINE_RESULT_TYPE(double);
DEFINE_RESULT_TYPE(int8_t);
DEFINE_RESULT_TYPE(uint8_t);
DEFINE_RESULT_TYPE(int16_t);
DEFINE_RESULT_TYPE(uint16_t);
DEFINE_RESULT_TYPE(int32_t);
DEFINE_RESULT_TYPE(uint32_t);
DEFINE_RESULT_TYPE(int64_t);
DEFINE_RESULT_TYPE(uint64_t);
DEFINE_RESULT_TYPE(size_t);

// Special case for void
typedef struct {
    ErrorResult Error;
} voidResult;

// Indicates that the function returns an error
#define CanReturnError(...) __VA_ARGS__##Result

// Helper functions for error creation and checking
static inline ErrorResult errorResultCreate(uint8_t code, const char* message) {
    ErrorResult result = {code, message};
    return result;
}

static inline int errorResultOccurred(ErrorResult error) {
    return error.Code != ERROR_CODE_NO_ERROR;
}

static inline const char* errorResultGetMessage(ErrorResult error) {
    return error.Message ? error.Message : "No error message provided";
}

#endif // LIBRARY_ERROR_RESULT_H
