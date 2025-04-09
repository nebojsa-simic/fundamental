#include "fileRead.h"

AsyncResult fun_read_file_in_memory(Read parameters)
{
	MMapState *state =
		(MMapState *)fun_memory_allocate(sizeof(MMapState)).value;
	*state = (MMapState){ .parameters = parameters };

	AsyncResult result = {
		.state = state,
		.poll = poll_mmap,
		.status = ASYNC_PENDING,
	};

	return result;
}