#include "fundamental/json/json.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define RED_CROSS "\033[0;31m✗\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

static void print_test_result(const char *test_name, int passed)
{
	if (passed) {
		printf("%s %s\n", GREEN_CHECK, test_name);
		tests_passed++;
	} else {
		printf("%s %s\n", RED_CROSS, test_name);
		tests_failed++;
	}
}

// === Init ===

static void test_init_null(void)
{
	char data[] = "{}";
	int ok = 0;

	ErrorResult r = fun_json_init(NULL, data, 2);
	if (fun_error_is_error(r))
		ok++;

	FunJsonState state;
	r = fun_json_init(&state, NULL, 2);
	if (fun_error_is_error(r))
		ok++;

	print_test_result("fun_json_init null args", ok == 2);
}

static void test_init_empty(void)
{
	char data[] = "";
	FunJsonState state;
	ErrorResult r = fun_json_init(&state, data, 0);
	assert(!fun_error_is_error(r));

	FunJsonToken token;
	r = fun_json_next(&state, &token);
	assert(!fun_error_is_error(r));
	print_test_result("fun_json_init empty input",
					  token.type == FUN_JSON_TOKEN_END);
}

static void test_init_valid(void)
{
	char data[] = "{}";
	FunJsonState state;
	ErrorResult r = fun_json_init(&state, data, 2);
	print_test_result("fun_json_init valid", !fun_error_is_error(r));
}

// === Token iteration ===

static void test_simple_object(void)
{
	char data[] = "{\"key\":\"value\"}";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;

	fun_json_next(&state, &t);
	int ok = (t.type == FUN_JSON_OBJECT_START && t.depth == 1);

	fun_json_next(&state, &t);
	ok = ok && (t.type == FUN_JSON_KEY && strcmp(t.value, "key") == 0);

	fun_json_next(&state, &t);
	ok = ok && (t.type == FUN_JSON_STRING && strcmp(t.value, "value") == 0);

	fun_json_next(&state, &t);
	ok = ok && (t.type == FUN_JSON_OBJECT_END);

	fun_json_next(&state, &t);
	ok = ok && (t.type == FUN_JSON_TOKEN_END);

	print_test_result("fun_json_next simple object", ok);
}

static void test_nested_array(void)
{
	char data[] = "[[1,2],[3]]";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	int ok = 1;

	fun_json_next(&state, &t);
	ok &= (t.type == FUN_JSON_ARRAY_START && t.depth == 1);

	fun_json_next(&state, &t);
	ok &= (t.type == FUN_JSON_ARRAY_START && t.depth == 2);

	fun_json_next(&state, &t);
	ok &= (t.type == FUN_JSON_NUMBER && t.depth == 2 && t.parent_is_array &&
		   t.array_index == 0);

	fun_json_next(&state, &t);
	ok &= (t.type == FUN_JSON_NUMBER && t.depth == 2 && t.array_index == 1);

	fun_json_next(&state, &t);
	ok &= (t.type == FUN_JSON_ARRAY_END);

	fun_json_next(&state, &t);
	ok &= (t.type == FUN_JSON_ARRAY_START && t.depth == 2);

	fun_json_next(&state, &t);
	ok &= (t.type == FUN_JSON_NUMBER && t.depth == 2 && t.parent_is_array &&
		   t.array_index == 0);

	print_test_result("fun_json_next nested array tracking", ok);
}

// === Primitive values ===

static void test_string_value(void)
{
	char data[] = "\"hello world\"";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t);

	int ok = (t.type == FUN_JSON_STRING && t.length == 11 &&
			  memcmp(t.value, "hello world", 11) == 0);
	print_test_result("fun_json_next string value", ok);
}

static void test_integer_value(void)
{
	char data[] = "42";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t);

	int ok = (t.type == FUN_JSON_NUMBER && t.length == 2 &&
			  memcmp(t.value, "42", 2) == 0);
	print_test_result("fun_json_next integer", ok);
}

static void test_negative_number(void)
{
	char data[] = "-17";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t);

	int ok = (t.type == FUN_JSON_NUMBER && t.length == 3 &&
			  memcmp(t.value, "-17", 3) == 0);
	print_test_result("fun_json_next negative number", ok);
}

static void test_float_number(void)
{
	char data[] = "3.14";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t);

	int ok = (t.type == FUN_JSON_NUMBER && t.length == 4 &&
			  memcmp(t.value, "3.14", 4) == 0);
	print_test_result("fun_json_next float", ok);
}

static void test_literals(void)
{
	char d1[] = "true";
	FunJsonState state;
	fun_json_init(&state, d1, 4);
	FunJsonToken t;
	fun_json_next(&state, &t);
	int ok = (t.type == FUN_JSON_BOOL);

	char d2[] = "false";
	fun_json_init(&state, d2, 5);
	fun_json_next(&state, &t);
	ok = ok && (t.type == FUN_JSON_BOOL);

	char d3[] = "null";
	fun_json_init(&state, d3, 4);
	fun_json_next(&state, &t);
	ok = ok && (t.type == FUN_JSON_NULL);

	print_test_result("fun_json_next literals", ok);
}

// === String escaping ===

static void test_string_escapes(void)
{
	char data[] = "\"he\\\"llo\"";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t);

	int ok = (t.type == FUN_JSON_STRING && t.length == 6 &&
			  memcmp(t.value, "he\"llo", 6) == 0);

	char data2[] = "\"a\\\\b\"";
	fun_json_init(&state, data2, fun_string_length(data2));
	fun_json_next(&state, &t);
	ok = ok && (t.type == FUN_JSON_STRING && t.length == 3 &&
				memcmp(t.value, "a\\b", 3) == 0);

	char data3[] = "\"line\\nbreak\"";
	fun_json_init(&state, data3, fun_string_length(data3));
	fun_json_next(&state, &t);
	ok = ok &&
		 (t.type == FUN_JSON_STRING && t.length == 10 && t.value[4] == '\n');

	print_test_result("fun_json_next string escapes", ok);
}

static void test_unicode_escape(void)
{
	char data[] = "\"\\u0041\"";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t);

	int ok = (t.type == FUN_JSON_STRING && t.length == 1 && t.value[0] == 'A');
	print_test_result("fun_json_next unicode escape", ok);
}

// === Error detection ===

static void test_error_unterminated_string(void)
{
	char data[] = "\"hello";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	ErrorResult r = fun_json_next(&state, &t);
	print_test_result("fun_json_next unterminated string",
					  fun_error_is_error(r));
}

static void test_error_invalid_number(void)
{
	char data[] = "01";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	ErrorResult r = fun_json_next(&state, &t);
	print_test_result("fun_json_next invalid number", fun_error_is_error(r));
}

static void test_error_unexpected_token(void)
{
	char data[] = "@@@";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	ErrorResult r = fun_json_next(&state, &t);
	print_test_result("fun_json_next unexpected token", fun_error_is_error(r));
}

// === Typed extractors ===

static void test_extract_int(void)
{
	char data[] = "42";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t);

	int64_tResult r = fun_json_token_as_int(&t);
	int ok = (!fun_error_is_error(r.error) && r.value == 42);
	print_test_result("fun_json_token_as_int", ok);
}

static void test_extract_double(void)
{
	char data[] = "3.14";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t);

	doubleResult r = fun_json_token_as_double(&t);
	int ok = (!fun_error_is_error(r.error) && r.value > 3.13 && r.value < 3.15);
	print_test_result("fun_json_token_as_double", ok);
}

static void test_extract_bool(void)
{
	char d1[] = "true";
	FunJsonState state;
	fun_json_init(&state, d1, 4);
	FunJsonToken t;
	fun_json_next(&state, &t);

	boolResult r = fun_json_token_as_bool(&t);
	int ok = (!fun_error_is_error(r.error) && r.value);

	char d2[] = "false";
	fun_json_init(&state, d2, 5);
	fun_json_next(&state, &t);
	r = fun_json_token_as_bool(&t);
	ok = ok && (!fun_error_is_error(r.error) && r.value);

	print_test_result("fun_json_token_as_bool", ok);
}

static void test_is_null(void)
{
	char d1[] = "null";
	FunJsonState state;
	fun_json_init(&state, d1, 4);
	FunJsonToken t;
	fun_json_next(&state, &t);

	boolResult r = fun_json_token_is_null(&t);
	int ok = (!fun_error_is_error(r.error) && r.value);

	char d2[] = "\"notnull\"";
	fun_json_init(&state, d2, fun_string_length(d2));
	fun_json_next(&state, &t);
	r = fun_json_token_is_null(&t);
	ok = ok && (!fun_error_is_error(r.error) && !r.value);

	print_test_result("fun_json_token_is_null", ok);
}

static void test_copy_string(void)
{
	char data[] = "\"hello world\"";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t);

	char out[32];
	ErrorResult r = fun_json_token_copy_string(&t, out, sizeof(out));
	int ok = (!fun_error_is_error(r) && strcmp(out, "hello world") == 0);
	print_test_result("fun_json_token_copy_string", ok);
}

// === Query ===

static void test_query_string(void)
{
	const char *data =
		"{\"apps\":{\"http\":{\"servers\":{\"restic\":{\"listen\":[\":443\"]}}}}}";

	FunJsonToken t;
	ErrorResult r =
		fun_json_query(data, fun_string_length(data), "apps.http", &t);
	int ok = (!fun_error_is_error(r) && t.type == FUN_JSON_OBJECT_START);

	print_test_result("fun_json_query object", ok);
}

static void test_query_deep_string(void)
{
	const char *data =
		"{\"apps\":{\"http\":{\"servers\":{\"restic\":{\"handler\":\"restic\"}}}}}";

	FunJsonToken t;
	ErrorResult r = fun_json_query(data, fun_string_length(data),
								   "apps.http.servers.restic.handler", &t);
	int ok = (!fun_error_is_error(r) && t.type == FUN_JSON_STRING &&
			  t.length == 6 && memcmp(t.value, "restic", 6) == 0);

	print_test_result("fun_json_query deep string", ok);
}

static void test_query_path_not_found(void)
{
	const char *data = "{\"a\":1}";
	FunJsonToken t;
	ErrorResult r = fun_json_query(data, fun_string_length(data), "b", &t);
	print_test_result("fun_json_query path not found", fun_error_is_error(r));
}

// === Convenience combinators ===

static void test_query_string_combinator(void)
{
	const char *data =
		"{\"name\":\"Alice\",\"age\":30,\"pi\":3.14,\"enabled\":true}";

	char out[64];
	ErrorResult r = fun_json_query_string(data, fun_string_length(data), "name",
										  out, sizeof(out));
	int ok = (!fun_error_is_error(r) && strcmp(out, "Alice") == 0);

	int64_tResult ir = fun_json_query_int(data, fun_string_length(data), "age");
	ok = ok && (!fun_error_is_error(ir.error) && ir.value == 30);

	doubleResult dr =
		fun_json_query_double(data, fun_string_length(data), "pi");
	ok = ok &&
		 (!fun_error_is_error(dr.error) && dr.value > 3.13 && dr.value < 3.15);

	boolResult br =
		fun_json_query_bool(data, fun_string_length(data), "enabled");
	ok = ok && (!fun_error_is_error(br.error) && br.value);

	print_test_result("fun_json_query_* combinators", ok);
}

// === Array extractors ===

static void test_int_array(void)
{
	const char *data = "{\"nodes\":[0,1,2,3]}";
	int64_t out[8];
	uint64_tResult r = fun_json_query_int_array(data, fun_string_length(data),
												"nodes", out, 8);

	int ok = (!fun_error_is_error(r.error) && r.value == 4 && out[0] == 0 &&
			  out[1] == 1 && out[2] == 2 && out[3] == 3);
	print_test_result("fun_json_query_int_array", ok);
}

static void test_double_array(void)
{
	const char *data = "{\"max\":[1.0,1.0,0.0]}";
	double out[4];
	uint64_tResult r = fun_json_query_double_array(
		data, fun_string_length(data), "max", out, 4);

	int ok = (!fun_error_is_error(r.error) && r.value == 3 && out[0] > 0.99 &&
			  out[1] > 0.99 && out[2] < 0.01);
	print_test_result("fun_json_query_double_array", ok);
}

static void test_string_array(void)
{
	const char *data = "{\"hosts\":[\"a.com\",\"b.com\",\"c.com\"]}";
	char buf[256];
	uint64_tResult r = fun_json_query_string_array(
		data, fun_string_length(data), "hosts", buf, sizeof(buf));

	int ok = (!fun_error_is_error(r.error) && r.value == 3);

	// Walk buffer: strings are \0-separated
	char *p = buf;
	ok = ok && (strcmp(p, "a.com") == 0);
	p += strlen(p) + 1;
	ok = ok && (strcmp(p, "b.com") == 0);
	p += strlen(p) + 1;
	ok = ok && (strcmp(p, "c.com") == 0);

	print_test_result("fun_json_query_string_array", ok);
}

static void test_empty_array(void)
{
	const char *data = "{\"empty\":[]}";
	int64_t out[4];
	uint64_tResult r = fun_json_query_int_array(data, fun_string_length(data),
												"empty", out, 4);

	print_test_result("fun_json_query_int_array empty",
					  !fun_error_is_error(r.error) && r.value == 0);
}

// === init_at_path ===

static void test_init_at_path(void)
{
	char data[] =
		"{\"apps\":{\"http\":{\"servers\":{\"restic\":{\"listen\":[\":443\"],\"routes\":[{}]}}}}}";
	FunJsonState state;
	uint64_t depth;
	ErrorResult r = fun_json_init_at_path(&state, data, fun_string_length(data),
										  "apps.http.servers.restic", &depth);

	int ok = !fun_error_is_error(r);

	FunJsonToken t;
	fun_json_next(&state, &t);
	ok = ok && (t.type == FUN_JSON_OBJECT_START);

	print_test_result("fun_json_init_at_path", ok);
}

// === Skip value ===

static void test_skip_value(void)
{
	char data[] = "{\"a\":{\"nested\":true},\"b\":42}";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t); // OBJECT_START
	fun_json_next(&state, &t); // KEY "a"

	// skip the value (nested object)
	fun_json_skip_value(&state);

	// next should be KEY "b"
	fun_json_next(&state, &t);
	int ok = (t.type == FUN_JSON_KEY && strcmp(t.value, "b") == 0);

	fun_json_next(&state, &t);
	ok = ok && (t.type == FUN_JSON_NUMBER);

	print_test_result("fun_json_skip_value", ok);
}

// === Depth helpers ===

static void test_next_at(void)
{
	char data[] = "{\"a\":{\"b\":{\"c\":1}}}";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t); // OBJECT_START
	fun_json_next(&state, &t); // KEY "a"

	// skip nested stuff, get next token at depth 1
	fun_json_next_at(&state, 1, &t);

	int ok = (t.type == FUN_JSON_OBJECT_END);

	print_test_result("fun_json_next_at", ok);
}

static void test_find_key(void)
{
	char data[] = "{\"x\":1,\"target\":\"found\",\"y\":2}";
	FunJsonState state;
	fun_json_init(&state, data, fun_string_length(data));

	FunJsonToken t;
	fun_json_next(&state, &t); // OBJECT_START

	ErrorResult r = fun_json_find_key(&state, 1, "target", &t);
	int ok = (!fun_error_is_error(r) && t.type == FUN_JSON_STRING &&
			  t.length == 5 && memcmp(t.value, "found", 5) == 0);

	print_test_result("fun_json_find_key", ok);
}

// === Non-mutating query idempotency ===

static void test_query_idempotent(void)
{
	const char *data = "{\"a\":\"val1\",\"b\":\"val2\",\"c\":\"val3\"}";
	uint64_t len = fun_string_length(data);

	char out1[64], out2[64];
	ErrorResult r1 = fun_json_query_string(data, len, "a", out1, sizeof(out1));
	ErrorResult r2 = fun_json_query_string(data, len, "b", out2, sizeof(out2));

	int ok = (!fun_error_is_error(r1) && !fun_error_is_error(r2) &&
			  strcmp(out1, "val1") == 0 && strcmp(out2, "val2") == 0);
	print_test_result("fun_json_query idempotent", ok);
}

int main(void)
{
	printf("=== JSON Module Tests ===\n\n");

	test_init_null();
	test_init_empty();
	test_init_valid();
	test_simple_object();
	test_nested_array();
	test_string_value();
	test_integer_value();
	test_negative_number();
	test_float_number();
	test_literals();
	test_string_escapes();
	test_unicode_escape();
	test_error_unterminated_string();
	test_error_invalid_number();
	test_error_unexpected_token();
	test_extract_int();
	test_extract_double();
	test_extract_bool();
	test_is_null();
	test_copy_string();
	test_query_string();
	test_query_deep_string();
	test_query_path_not_found();
	test_query_string_combinator();
	test_int_array();
	test_double_array();
	test_string_array();
	test_empty_array();
	test_init_at_path();
	test_skip_value();
	test_next_at();
	test_find_key();
	test_query_idempotent();

	printf("\nResults: %d passed, %d failed, %d total\n", tests_passed,
		   tests_failed, tests_passed + tests_failed);
	return tests_failed > 0 ? 1 : 0;
}
