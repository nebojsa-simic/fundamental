// glTF Parser Demo — Fundamental Library
// Parses Sponza.gltf (167KB, 405 accessors, 103 primitives)
// Uses JSON Layer 2 for counts, Layer 1 for per-primitive stats

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

static void print_count(const char *label, int64_t count)
{
	fun_console_write("  ");
	fun_console_write(label);
	fun_console_write(": ");
	print_int(count);
	fun_console_write_line("");
}

static int64_t json_count_array_elements(char *data, uint64_t len, String path)
{
	FunJsonToken arr;
	ErrorResult err = fun_json_query(data, len, path, &arr);
	if (fun_error_is_error(err) || arr.type != FUN_JSON_ARRAY_START)
		return -1;

	FunJsonState s;
	s._data = data;
	s._pos = arr.value ? (uint64_t)(arr.value - data) + 1 : 1;
	s._len = len;
	s._depth = arr.depth;
	s._in_array[s._depth] = true;
	s._array_index[s._depth] = 0;
	s._expecting_value[s._depth] = true;
	s._expecting_key[s._depth] = false;
	s._expecting_comma[s._depth] = false;
	s._mutating = false;

	int64_t count = 0;
	FunJsonToken t;
	while (1) {
		err = fun_json_next_at(&s, arr.depth + 1, &t);
		if (fun_error_is_error(err))
			break;
		if (t.type == FUN_JSON_TOKEN_END)
			break;
		if (t.type == FUN_JSON_ARRAY_END && t.depth <= arr.depth)
			break;
		if (t.depth == arr.depth + 1 && t.type == FUN_JSON_OBJECT_START)
			count++;
	}
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

	// --- Layer 2: top-level queries ---

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

	// --- Accessor types summary (non-mutating) ---
	fun_console_write_line("--- Accessor Types ---");
	int64_t scalar_count = 0, vec2_count = 0, vec3_count = 0, vec4_count = 0,
			mat_count = 0;

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

			if (type_tok.length == 6 && type_tok.value[0] == 'S')
				scalar_count++;
			else if (type_tok.length == 4 && type_tok.value[0] == 'V' &&
					 type_tok.value[3] == '2')
				vec2_count++;
			else if (type_tok.length == 4 && type_tok.value[0] == 'V' &&
					 type_tok.value[3] == '3')
				vec3_count++;
			else if (type_tok.length == 4 && type_tok.value[0] == 'V' &&
					 type_tok.value[3] == '4')
				vec4_count++;
			else
				mat_count++;
		}
	}

	print_count("SCALAR", scalar_count);
	print_count("VEC2", vec2_count);
	print_count("VEC3", vec3_count);
	print_count("VEC4", vec4_count);
	print_count("MATn", mat_count);
	fun_console_write_line("");

	// --- First accessor detail (non-mutating) ---
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
