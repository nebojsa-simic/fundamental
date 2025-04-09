#pragma once

#include "fileRead.h"

static DWORD get_allocation_granularity()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwAllocationGranularity;
}

static AsyncStatus poll_mmap(AsyncResult *result)
{
	MMapState *state = (MMapState *)result->state;

	if (!state->file_handle) {
		// Step 1: Open file
		state->file_handle = CreateFileW(state->parameters.file_path,
										 GENERIC_READ, FILE_SHARE_READ, NULL,
										 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
										 NULL);

		if (state->file_handle == INVALID_HANDLE_VALUE) {
			// result->error = (ErrorResult) ;
			return ASYNC_ERROR;
		}
		return ASYNC_PENDING;
	}

	if (!state->mapping_handle) {
		// Step 2: Create file mapping
		state->mapping_handle = CreateFileMappingW(state->file_handle, NULL,
												   PAGE_READONLY, 0, 0, NULL);

		if (!state->mapping_handle) {
			// result->error = get_system_error(GetLastError());
			CloseHandle(state->file_handle);
			return ASYNC_ERROR;
		}
		return ASYNC_PENDING;
	}

	if (!state->adjusted_offset) {
		// Step 3: Calculate aligned offset
		DWORD granularity = get_allocation_granularity();
		state->adjusted_offset =
			state->parameters.offset / granularity * granularity;
		return ASYNC_PENDING;
	}

	if (!state->mapped_view) {
		// Step 4: Map view of file
		DWORD offset_high = (DWORD)(state->adjusted_offset >> 32);
		DWORD offset_low = (DWORD)(state->adjusted_offset & 0xFFFFFFFF);
		SIZE_T view_size = state->parameters.bytes_to_read +
						   (state->parameters.offset - state->adjusted_offset);

		state->mapped_view = MapViewOfFile(state->mapping_handle, FILE_MAP_READ,
										   offset_high, offset_low, view_size);

		if (!state->mapped_view) {
			// result->error = get_system_error(GetLastError());
			CloseHandle(state->mapping_handle);
			CloseHandle(state->file_handle);
			return ASYNC_ERROR;
		}
		return ASYNC_PENDING;
	}

	// Step 5: Copy data and cleanup
	BYTE *data_ptr = (BYTE *)state->mapped_view +
					 (state->parameters.offset - state->adjusted_offset);
	// Use memory module for allocation and copying

	fun_memory_copy(data_ptr, state->parameters.output,
					state->parameters.bytes_to_read);

	// Store final buffer and cleanup
	UnmapViewOfFile(state->mapped_view);
	CloseHandle(state->mapping_handle);
	CloseHandle(state->file_handle);

	fun_memory_free(state);

	return ASYNC_COMPLETED;
}
