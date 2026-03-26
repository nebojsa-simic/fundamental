#include "fundamental/tsv/tsv.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define RED_CROSS "\033[0;31m✗\033[0m"

static void print_test_result(const char *test_name, int passed)
{
	if (passed) {
		printf("%s %s\n", GREEN_CHECK, test_name);
	} else {
		printf("%s %s\n", RED_CROSS, test_name);
	}
}

// test_fun_tsv_single_row: parse "F\tfile.txt\n", verify count==2
static void test_fun_tsv_single_row(void)
{
	char data[] = "F\tfile.txt\n";
	FunTsvState state;
	ErrorResult init_err = fun_tsv_init(&state, data);
	if (fun_error_is_error(init_err)) {
		print_test_result("fun_tsv_single_row", 0);
		return;
	}

	FunTsvRow row;
	boolResult r = fun_tsv_next(&state, &row);

	int passed =
		(r.value == true && row.count == 2 && strcmp(row.fields[0], "F") == 0 &&
		 strcmp(row.fields[1], "file.txt") == 0);
	print_test_result("fun_tsv_single_row", passed);
}

// test_fun_tsv_multiple_rows: parse 3-row listing, verify 3 rows
static void test_fun_tsv_multiple_rows(void)
{
	char data[] = "D\tsub\nF\ta.c\nF\tb.c\n";
	FunTsvState state;
	fun_tsv_init(&state, data);

	FunTsvRow row;
	int row_count = 0;
	int all_ok = 1;

	boolResult r = fun_tsv_next(&state, &row);
	if (!r.value || row.count != 2 || strcmp(row.fields[0], "D") != 0 ||
		strcmp(row.fields[1], "sub") != 0) {
		all_ok = 0;
	}
	row_count++;

	r = fun_tsv_next(&state, &row);
	if (!r.value || row.count != 2 || strcmp(row.fields[0], "F") != 0 ||
		strcmp(row.fields[1], "a.c") != 0) {
		all_ok = 0;
	}
	row_count++;

	r = fun_tsv_next(&state, &row);
	if (!r.value || row.count != 2 || strcmp(row.fields[0], "F") != 0 ||
		strcmp(row.fields[1], "b.c") != 0) {
		all_ok = 0;
	}
	row_count++;

	r = fun_tsv_next(&state, &row);
	if (r.value) {
		all_ok = 0; // Should be end of data
	}

	print_test_result("fun_tsv_multiple_rows", all_ok && row_count == 3);
}

// test_fun_tsv_empty: init on "", first call returns false
static void test_fun_tsv_empty(void)
{
	char data[] = "";
	FunTsvState state;
	fun_tsv_init(&state, data);

	FunTsvRow row;
	boolResult r = fun_tsv_next(&state, &row);

	print_test_result("fun_tsv_empty", r.value == false);
}

// test_fun_tsv_extra_columns: parse "D\tsubdir\t12345\n", verify count==3
static void test_fun_tsv_extra_columns(void)
{
	char data[] = "D\tsubdir\t12345\n";
	FunTsvState state;
	fun_tsv_init(&state, data);

	FunTsvRow row;
	boolResult r = fun_tsv_next(&state, &row);

	int passed =
		(r.value == true && row.count == 3 && strcmp(row.fields[0], "D") == 0 &&
		 strcmp(row.fields[1], "subdir") == 0 &&
		 strcmp(row.fields[2], "12345") == 0);
	print_test_result("fun_tsv_extra_columns", passed);
}

// test_fun_tsv_null: null state returns error
static void test_fun_tsv_null(void)
{
	// NULL state to fun_tsv_init
	char data[] = "F\tfile.txt\n";
	ErrorResult err = fun_tsv_init(NULL, data);
	int init_null_ok = fun_error_is_error(err);

	// NULL data to fun_tsv_init
	FunTsvState state;
	err = fun_tsv_init(&state, NULL);
	int data_null_ok = fun_error_is_error(err);

	// NULL state to fun_tsv_next
	FunTsvRow row;
	boolResult r = fun_tsv_next(NULL, &row);
	int next_null_ok = fun_error_is_error(r.error);

	print_test_result("fun_tsv_null",
					  init_null_ok && data_null_ok && next_null_ok);
}

int main(void)
{
	printf("=== TSV Module Tests ===\n\n");

	test_fun_tsv_single_row();
	test_fun_tsv_multiple_rows();
	test_fun_tsv_empty();
	test_fun_tsv_extra_columns();
	test_fun_tsv_null();

	printf("\nTests completed.\n");
	return 0;
}
