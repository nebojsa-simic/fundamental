#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
	Write parameters;
	int file_descriptor;
	bool fd_valid;
	void *mapped_address;
	bool mmap_valid;
	uint64_t adjusted_offset;
	uint64_t original_file_size;
	bool file_extended;
} MMapWriteState;

typedef struct {
	Write parameters;
	int ring_fd;
	int file_fd;
	void *sq_ring;
	void *cq_ring;
	bool ring_initialized;
	bool file_opened;
	bool io_submitted;
	bool cqe_consumed;
} RingWriteState;

AsyncStatus poll_mmap_write(AsyncResult *result);
AsyncResult create_ring_write(Write parameters);
