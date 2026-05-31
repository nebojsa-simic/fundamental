// glTF Parser Demo — Fundamental Library
// Parses Sponza.gltf (167KB, 405 accessors, 103 primitives)

#include "fundamental/console/console.h"
#include "fundamental/string/string.h"
#include "fundamental/memory/memory.h"
#include "fundamental/file/file.h"
#include "fundamental/async/async.h"
#include "fundamental/json/json.h"

#define FILE_BUF_SIZE 167176

static void print_int(int64_t num)
{
	char buf[32];
	fun_string_from_int(num, 10, buf, sizeof(buf));
	fun_console_write(buf);
}

static ErrorResult count_accessor_type(FunJsonToken *element, uint64_t idx,
									   void *ctx)
{
	(void)idx;
	int64_t *counts = (int64_t *)ctx;
	if (fun_json_token_value_equals(element, "SCALAR"))
		counts[0]++;
	else if (fun_json_token_value_equals(element, "VEC2"))
		counts[1]++;
	else if (fun_json_token_value_equals(element, "VEC3"))
		counts[2]++;
	else if (fun_json_token_value_equals(element, "VEC4"))
		counts[3]++;
	else
		counts[4]++;
	return ERROR_RESULT_NO_ERROR;
}

static void print_count(const char *label, int64_t count)
{
	fun_console_write("  ");
	fun_console_write(label);
	fun_console_write(": ");
	print_int(count);
	fun_console_write_line("");
}

static ErrorResult json_count_cb(FunJsonToken *element, uint64_t idx, void *ctx)
{
	(void)idx;
	(void)element;
	int64_t *count = (int64_t *)ctx;
	(*count)++;
	return ERROR_RESULT_NO_ERROR;
}

static int64_t json_count_array_elements(char *data, uint64_t len, String path)
{
	int64_t count = 0;
	fun_json_for_each(data, len, path, json_count_cb, &count);
	return count;
}

int main(void)
{
	// --- Load glTF file ---
	MemoryResult mem = fun_memory_allocate(FILE_BUF_SIZE);
	if (fun_error_is_error(mem.error)) {
		fun_console_write_line("ERR: memory allocation failed");
		return 1;
	}
	char *data = (char *)mem.value;

	AsyncResult read = fun_read_file_in_memory((Read){
		.file_path = "Sponza.gltf",
		.output = mem.value,
		.bytes_to_read = FILE_BUF_SIZE,
		.offset = 0,
	});
	fun_async_await(&read, -1);

	if (read.status != ASYNC_COMPLETED) {
		fun_console_write_line("ERR: file read failed");
		(void)fun_memory_free(&mem.value);
		return 1;
	}

	uint64_t len = fun_string_length(data);

	fun_console_write_line("=== Sponza glTF Parser ===");
	fun_console_write_line("");

	// --- Layer 2: top-level counts ---
	fun_console_write_line("Top-level counts:");
	print_count("meshes", json_count_array_elements(data, len, "meshes"));
	print_count("nodes", json_count_array_elements(data, len, "nodes"));
	print_count("accessors", json_count_array_elements(data, len, "accessors"));
	print_count("materials", json_count_array_elements(data, len, "materials"));
	print_count("textures", json_count_array_elements(data, len, "textures"));
	fun_console_write_line("");

	// --- Asset info ---
	char version[16];
	fun_json_query_string(data, len, "asset.version", version, sizeof(version));
	char generator[64];
	fun_json_query_string(data, len, "asset.generator", generator,
						  sizeof(generator));

	fun_console_write("Asset: glTF ");
	fun_console_write(version);
	fun_console_write(" (");
	fun_console_write(generator);
	fun_console_write_line(")");
	fun_console_write_line("");

	// --- Accessor types via for_each ---
	fun_console_write_line("--- Accessor Types ---");
	int64_t type_counts[5] = { 0, 0, 0, 0, 0 };

	FunJsonToken arr_token;
	ErrorResult err = fun_json_query(data, len, "accessors", &arr_token);
	if (!fun_error_is_error(err) && arr_token.type == FUN_JSON_ARRAY_START) {
		FunJsonState as;
		as._data = data;
		as._pos = arr_token.value ? (uint64_t)(arr_token.value - data) + 1 : 1;
		as._len = len;
		as._depth = arr_token.depth;
		as._in_array[arr_token.depth] = true;
		as._array_index[arr_token.depth] = 0;
		as._expecting_value[arr_token.depth] = true;
		as._expecting_comma[arr_token.depth] = false;
		as._expecting_key[arr_token.depth] = false;
		as._mutating = false;

		FunJsonToken at;
		while (1) {
			err = fun_json_next_at(&as, arr_token.depth + 1, &at);
			if (fun_error_is_error(err))
				break;
			if (at.type == FUN_JSON_TOKEN_END)
				break;
			if (at.type == FUN_JSON_ARRAY_END && at.depth <= arr_token.depth)
				break;
			if (at.type != FUN_JSON_OBJECT_START)
				continue;

			FunJsonToken type_tok;
			err =
				fun_json_find_key(&as, arr_token.depth + 1, "type", &type_tok);
			if (fun_error_is_error(err))
				continue;
			(void)count_accessor_type(&type_tok, 0, type_counts);
		}
	}

	print_count("SCALAR", type_counts[0]);
	print_count("VEC2", type_counts[1]);
	print_count("VEC3", type_counts[2]);
	print_count("VEC4", type_counts[3]);
	print_count("MATn", type_counts[4]);
	fun_console_write_line("");

	// --- First accessor detail ---
	fun_console_write_line("--- First Accessor ---");
	{
		char buf[64];
		FunJsonToken acc;
		ErrorResult er = fun_json_query(data, len, "accessors.0.type", &acc);
		if (!fun_error_is_error(er)) {
			(void)fun_json_token_copy_string(&acc, buf, sizeof(buf));
			fun_console_write("  type: ");
			fun_console_write(buf);
			fun_console_write_line("");
		}

		FunJsonToken ac;
		er = fun_json_query(data, len, "accessors.0.count", &ac);
		if (!fun_error_is_error(er)) {
			int64_t cnt = fun_json_token_as_int(&ac).value;
			fun_console_write("  count: ");
			print_int(cnt);
			fun_console_write_line("");
		}
	}

	// --- Cleanup ---
	(void)fun_memory_free(&mem.value);

	fun_console_write_line("");
	fun_console_write_line("Done.");
	return 0;
}
