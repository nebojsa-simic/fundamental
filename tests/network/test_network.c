#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../include/network/network.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"
#define ASSERT_NO_ERROR(result) assert((result).error.code == 0)
#define ASSERT_ERROR(result) assert((result).error.code != 0)

static void print_test_result(const char *name)
{
	printf("%s %s\n", GREEN_CHECK, name);
}

/* ================================================================
 * NetworkAddress parse tests (5.1)
 * ================================================================ */

static void test_fun_network_address_parse_ipv4_valid(void)
{
	NetworkAddressResult r = fun_network_address_parse("127.0.0.1:8080");
	ASSERT_NO_ERROR(r);
	assert(r.value.family == NETWORK_ADDRESS_IPV4);
	assert(r.value.bytes[0] == 127);
	assert(r.value.bytes[1] == 0);
	assert(r.value.bytes[2] == 0);
	assert(r.value.bytes[3] == 1);
	assert(r.value.port == 8080);
	print_test_result("fun_network_address_parse: valid IPv4");
}

static void test_fun_network_address_parse_ipv4_all_octets(void)
{
	NetworkAddressResult r = fun_network_address_parse("192.168.1.255:443");
	ASSERT_NO_ERROR(r);
	assert(r.value.bytes[0] == 192);
	assert(r.value.bytes[1] == 168);
	assert(r.value.bytes[2] == 1);
	assert(r.value.bytes[3] == 255);
	assert(r.value.port == 443);
	print_test_result("fun_network_address_parse: IPv4 all octets");
}

static void test_fun_network_address_parse_invalid_octet(void)
{
	NetworkAddressResult r = fun_network_address_parse("999.0.0.1:80");
	ASSERT_ERROR(r);
	print_test_result("fun_network_address_parse: rejects octet > 255");
}

static void test_fun_network_address_parse_missing_port(void)
{
	NetworkAddressResult r = fun_network_address_parse("127.0.0.1");
	ASSERT_ERROR(r);
	print_test_result("fun_network_address_parse: rejects missing port");
}

static void test_fun_network_address_parse_port_zero(void)
{
	NetworkAddressResult r = fun_network_address_parse("0.0.0.0:0");
	ASSERT_NO_ERROR(r);
	assert(r.value.port == 0);
	print_test_result("fun_network_address_parse: port 0 accepted");
}

static void test_fun_network_address_parse_port_max(void)
{
	NetworkAddressResult r = fun_network_address_parse("1.2.3.4:65535");
	ASSERT_NO_ERROR(r);
	assert(r.value.port == 65535);
	print_test_result("fun_network_address_parse: port 65535 accepted");
}

static void test_fun_network_address_parse_port_too_large(void)
{
	NetworkAddressResult r = fun_network_address_parse("1.2.3.4:65536");
	ASSERT_ERROR(r);
	print_test_result("fun_network_address_parse: rejects port > 65535");
}

static void test_fun_network_address_parse_hostname_rejected(void)
{
	NetworkAddressResult r = fun_network_address_parse("example.com:80");
	ASSERT_ERROR(r);
	print_test_result("fun_network_address_parse: rejects hostname");
}

static void test_fun_network_address_parse_localhost_rejected(void)
{
	NetworkAddressResult r = fun_network_address_parse("localhost:8080");
	ASSERT_ERROR(r);
	print_test_result("fun_network_address_parse: rejects 'localhost'");
}

static void test_fun_network_address_parse_null(void)
{
	NetworkAddressResult r = fun_network_address_parse(NULL);
	ASSERT_ERROR(r);
	print_test_result("fun_network_address_parse: rejects NULL");
}

static void test_fun_network_address_parse_empty(void)
{
	NetworkAddressResult r = fun_network_address_parse("");
	ASSERT_ERROR(r);
	print_test_result("fun_network_address_parse: rejects empty string");
}

static void test_fun_network_address_parse_ipv6_loopback(void)
{
	NetworkAddressResult r = fun_network_address_parse("[::1]:9000");
	ASSERT_NO_ERROR(r);
	assert(r.value.family == NETWORK_ADDRESS_IPV6);
	assert(r.value.port == 9000);
	/* ::1 = 15 zero bytes + 0x01 */
	for (int i = 0; i < 15; i++)
		assert(r.value.bytes[i] == 0);
	assert(r.value.bytes[15] == 1);
	print_test_result("fun_network_address_parse: IPv6 loopback [::1]");
}

static void test_fun_network_address_parse_ipv6_missing_bracket(void)
{
	NetworkAddressResult r = fun_network_address_parse("[::1:80");
	ASSERT_ERROR(r);
	print_test_result("fun_network_address_parse: rejects missing ]");
}

/* ================================================================
 * NetworkAddress to_string tests (5.2)
 * ================================================================ */

static void test_fun_network_address_to_string_ipv4_roundtrip(void)
{
	NetworkAddressResult parse = fun_network_address_parse("10.0.0.1:3000");
	ASSERT_NO_ERROR(parse);

	char buf[64];
	voidResult fmt =
		fun_network_address_to_string(parse.value, buf, sizeof(buf));
	ASSERT_NO_ERROR(fmt);
	assert(strcmp(buf, "10.0.0.1:3000") == 0);
	print_test_result("fun_network_address_to_string: IPv4 roundtrip");
}

static void test_fun_network_address_to_string_buffer_too_small(void)
{
	NetworkAddressResult parse = fun_network_address_parse("127.0.0.1:8080");
	ASSERT_NO_ERROR(parse);

	char tiny[5];
	voidResult fmt =
		fun_network_address_to_string(parse.value, tiny, sizeof(tiny));
	ASSERT_ERROR(fmt);
	print_test_result("fun_network_address_to_string: rejects tiny buffer");
}

/* ================================================================
 * NetworkBuffer slice tests (5.3)
 * ================================================================ */

static void test_fun_network_buffer_slice_valid(void)
{
	char data[] = "0123456789";
	NetworkBuffer buf;
	buf.data = data;
	buf.length = 10;

	NetworkBufferResult r = fun_network_buffer_slice(buf, 3, 4);
	ASSERT_NO_ERROR(r);
	assert(r.value.length == 4);
	assert(memcmp(r.value.data, "3456", 4) == 0);
	print_test_result("fun_network_buffer_slice: valid slice");
}

static void test_fun_network_buffer_slice_out_of_bounds(void)
{
	char data[] = "0123456789";
	NetworkBuffer buf;
	buf.data = data;
	buf.length = 10;

	NetworkBufferResult r = fun_network_buffer_slice(buf, 8, 5);
	ASSERT_ERROR(r);
	print_test_result("fun_network_buffer_slice: rejects out-of-bounds");
}

static void test_fun_network_buffer_slice_full(void)
{
	char data[] = "abcd";
	NetworkBuffer buf;
	buf.data = data;
	buf.length = 4;

	NetworkBufferResult r = fun_network_buffer_slice(buf, 0, 4);
	ASSERT_NO_ERROR(r);
	assert(r.value.length == 4);
	assert(memcmp(r.value.data, "abcd", 4) == 0);
	print_test_result("fun_network_buffer_slice: full-range slice");
}

/* ================================================================
 * NetworkBufferVector total length tests (5.4)
 * ================================================================ */

static void test_fun_network_buffer_vector_total_length_multi(void)
{
	char a[100], b[200], c[300];
	NetworkBuffer bufs[3];
	bufs[0].data = a;
	bufs[0].length = 100;
	bufs[1].data = b;
	bufs[1].length = 200;
	bufs[2].data = c;
	bufs[2].length = 300;
	NetworkBufferVector vec;
	vec.buffers = bufs;
	vec.count = 3;

	size_t total = fun_network_buffer_vector_total_length(vec);
	assert(total == 600);
	print_test_result(
		"fun_network_buffer_vector_total_length: sums 3 segments");
}

static void test_fun_network_buffer_vector_total_length_empty(void)
{
	NetworkBufferVector vec;
	vec.buffers = NULL;
	vec.count = 0;

	size_t total = fun_network_buffer_vector_total_length(vec);
	assert(total == 0);
	print_test_result(
		"fun_network_buffer_vector_total_length: empty vector = 0");
}

static void test_fun_network_buffer_vector_total_length_single(void)
{
	char data[42];
	NetworkBuffer buf;
	buf.data = data;
	buf.length = 42;
	NetworkBufferVector vec;
	vec.buffers = &buf;
	vec.count = 1;

	assert(fun_network_buffer_vector_total_length(vec) == 42);
	print_test_result("fun_network_buffer_vector_total_length: single segment");
}

/* ================================================================
 * main
 * ================================================================ */

int main(void)
{
	/* Address parse */
	test_fun_network_address_parse_ipv4_valid();
	test_fun_network_address_parse_ipv4_all_octets();
	test_fun_network_address_parse_invalid_octet();
	test_fun_network_address_parse_missing_port();
	test_fun_network_address_parse_port_zero();
	test_fun_network_address_parse_port_max();
	test_fun_network_address_parse_port_too_large();
	test_fun_network_address_parse_hostname_rejected();
	test_fun_network_address_parse_localhost_rejected();
	test_fun_network_address_parse_null();
	test_fun_network_address_parse_empty();
	test_fun_network_address_parse_ipv6_loopback();
	test_fun_network_address_parse_ipv6_missing_bracket();

	/* Address to_string */
	test_fun_network_address_to_string_ipv4_roundtrip();
	test_fun_network_address_to_string_buffer_too_small();

	/* Buffer slice */
	test_fun_network_buffer_slice_valid();
	test_fun_network_buffer_slice_out_of_bounds();
	test_fun_network_buffer_slice_full();

	/* BufferVector total length */
	test_fun_network_buffer_vector_total_length_multi();
	test_fun_network_buffer_vector_total_length_empty();
	test_fun_network_buffer_vector_total_length_single();

	printf("\nAll network tests passed.\n");
	return 0;
}
