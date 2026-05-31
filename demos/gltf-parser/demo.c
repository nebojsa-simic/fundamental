// glTF Parser Demo — Streaming via Fundamental Library
// Reads Sponza.gltf (167KB) in 4KB chunks, parses incrementally

#include "fundamental/console/console.h"
#include "fundamental/string/string.h"
#include "fundamental/memory/memory.h"
#include "fundamental/stream/stream.h"
#include "fundamental/async/async.h"
#include "fundamental/json/json.h"

#define BUF_SIZE 4096

static void print_int(int64_t num)
{
	char buf[32];
	fun_string_from_int(num, 10, buf, sizeof(buf));
	fun_console_write(buf);
}

static void print_count(const char *label, int64_t count)
{
	char buf[64];
	StringTemplateParam params[] = {
		{ .key = "label", .value = { .stringValue = label } },
		{ .key = "count", .value = { .intValue = count } },
	};
	fun_string_template("  ${label}: #{count}", params, 2, buf, sizeof(buf));
	fun_console_write_line(buf);
}

int main(void)
{
	// --- Allocate buffer for stream + JSON parser ---
	MemoryResult mem = fun_memory_allocate(BUF_SIZE);
	if (fun_error_is_error(mem.error)) {
		fun_console_write_line("ERR: memory allocation failed");
		return 1;
	}
	char *data = (char *)mem.value;

	// --- Open file stream ---
	AsyncResult open =
		fun_stream_create_file_read("Sponza.gltf", mem.value, BUF_SIZE, 0);
	fun_async_await(&open, -1);

	if (open.status != ASYNC_COMPLETED) {
		fun_console_write_line("ERR: failed to open file");
		(void)fun_memory_free(&mem.value);
		return 1;
	}
	void *stream = open.state;

	// --- Init JSON parser on empty buffer ---
	FunJsonState state;
	fun_json_init(&state, data, 0);

	fun_console_write_line("=== Streaming glTF Parser ===");
	fun_console_write_line("");

	int64_t obj_starts = 0, obj_ends = 0, keys = 0, strings = 0, numbers = 0;
	int64_t scalar_count = 0, vec2_count = 0, vec3_count = 0, vec4_count = 0;
	int64_t chunks = 0;
	bool type_key_active = false;
	bool in_accessors = false;
	int64_t accessor_depth = 0;

	// --- Read + parse loop ---
	while (1) {
		uint64_t bytes_read;
		AsyncResult read_result = fun_stream_read(stream, &bytes_read);
		fun_async_await(&read_result, -1);

		if (bytes_read == 0 || read_result.status != ASYNC_COMPLETED)
			break;
		chunks++;

		uint64_t new_len = state._len + bytes_read;
		fun_json_feed(&state, new_len);

		FunJsonToken t;
		while (1) {
			ErrorResult err = fun_json_next(&state, &t);
			if (fun_error_is_error(err))
				break;
			if (t.type == FUN_JSON_INCOMPLETE)
				break;
			if (t.type == FUN_JSON_TOKEN_END)
				break;

			switch (t.type) {
			case FUN_JSON_OBJECT_START:
				obj_starts++;
				break;
			case FUN_JSON_OBJECT_END:
				obj_ends++;
				break;
			case FUN_JSON_KEY:
				keys++;
				if (t.length == 4 && t.value[0] == 't' && t.value[1] == 'y' &&
					t.value[2] == 'p' && t.value[3] == 'e')
					type_key_active = true;
				else
					type_key_active = false;
				break;
			case FUN_JSON_STRING:
				strings++;
				if (type_key_active) {
					if (fun_json_token_value_equals(&t, "SCALAR"))
						scalar_count++;
					else if (fun_json_token_value_equals(&t, "VEC2"))
						vec2_count++;
					else if (fun_json_token_value_equals(&t, "VEC3"))
						vec3_count++;
					else if (fun_json_token_value_equals(&t, "VEC4"))
						vec4_count++;
				}
				type_key_active = false;
				break;
			case FUN_JSON_NUMBER:
				numbers++;
				break;
			default:
				break;
			}
		}
	}

	// --- Cleanup ---
	fun_stream_destroy(stream);
	(void)fun_memory_free(&mem.value);

	fun_console_write_line("Token counts:");
	print_count("objects (starts)", obj_starts);
	print_count("keys", keys);
	print_count("strings", strings);
	print_count("numbers", numbers);
	print_count("chunks read", chunks);
	fun_console_write_line("");

	fun_console_write_line("Accessor types (from streaming):");
	print_count("SCALAR", scalar_count);
	print_count("VEC2", vec2_count);
	print_count("VEC3", vec3_count);
	print_count("VEC4", vec4_count);
	fun_console_write_line("");

	fun_console_write_line("Done.");
	return 0;
}
