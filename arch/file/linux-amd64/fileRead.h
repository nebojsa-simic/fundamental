#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
	Read parameters;
	int file_descriptor;
	bool fd_valid;
	void *mapped_address;
	bool mmap_valid;
	uint64_t adjusted_offset;
} MMapState;

typedef struct {
	Read parameters;
	int ring_fd;
	int file_fd;
	void *sq_ring;
	void *cq_ring;
	uint32_t ring_mask;
	uint32_t ring_entries;
	bool ring_initialized;
	bool file_opened;
	bool io_submitted;
	bool cqe_consumed;
	void *mapped_buffer;
} RingReadState;

AsyncStatus poll_mmap(AsyncResult *result);
AsyncResult create_ring_read(Read parameters);
