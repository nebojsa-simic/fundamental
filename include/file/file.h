#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "../async/async.h"
#include "../memory/memory.h"      // Assumes Memory and String are defined in this header
#include "../string/string.h"
 
// Unified file access modes for different I/O implementations.
typedef enum {
    FILE_MODE_AUTO,       // Internally select best I/O mode using heuristics
    FILE_MODE_STDIO,      // Standard synchronous I/O
    FILE_MODE_MMAP,       // Memory-mapped I/O
    FILE_MODE_RING_BASED, // Ring-based asynchronous I/O (e.g., io_uring or I/O Rings)
    FILE_MODE_DIRECT      // Direct (unbuffered) I/O
} FileMode;

// Unified file handle structure
typedef struct {
    FileMode mode;      // The I/O mode selected for this file
    void* impl_state;   // Internal, implementation-specific state (opaque)
    uint64_t size;        // File size in bytes
    ErrorResult error;  // Last error, if any
} FunFile;

// I/O profile for heuristic-based mode selection (internal only)
typedef struct {
    uint64_t avg_offset_delta;   // Average offset change between consecutive operations
    uint64_t working_set_size;   // Estimated working set size
    uint32_t iops_counter; // Estimated I/O operations per second
    bool is_sequential;        // True if operations are largely sequential
} FunIoProfile;

// Unified file operations

/**
 * Opens a file with the specified path and mode.
 * 
 * @param path The path to the file (String type).
 * @param mode The desired FileMode (e.g., FILE_MODE_AUTO).
 * @param file Pointer to a FunFile structure to initialize.
 * @return ErrorResult indicating success or failure.
 */
ErrorResult fun_file_open(String path, FileMode mode, FunFile* file);

/**
 * Closes a previously opened file.
 *
 * @param file Pointer to a FunFile structure.
 */
void fun_file_close(FunFile* file);

/**
 * Reads data asynchronously from a file.
 *
 * @param file Pointer to the FunFile structure.
 * @param buffer Caller-allocated Memory buffer to store the read data.
 * @param count Number of bytes to read.
 * @param offset Offset in the file from which to start reading.
 * @return AsyncResult structure representing the asynchronous operation.
 */
AsyncResult fun_file_read(FunFile* file, Memory buffer, uint64_t count, uint64_t offset);

/**
 * Writes data asynchronously to a file.
 *
 * @param file Pointer to the FunFile structure.
 * @param buffer Caller-allocated Memory buffer containing the data to write.
 * @param count Number of bytes to write.
 * @param offset Offset in the file at which to start writing.
 * @return AsyncResult structure representing the asynchronous operation.
 */
AsyncResult fun_file_write(FunFile* file, Memory buffer, uint64_t count, uint64_t offset);
