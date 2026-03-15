/*
 * Tests for the network module — async TCP/UDP interface.
 *
 * stdio.h and assert.h are permitted in test files.
 */

#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "../../include/network/network.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"
#define ASSERT_NO_ERROR(r) assert((r).error.code == 0)
#define ASSERT_ERROR(r)    assert((r).error.code != 0)

static void print_ok(const char *name)
{
	printf("%s %s\n", GREEN_CHECK, name);
}

/* ================================================================
 * Platform loopback server helpers
 * ================================================================ */

#ifdef _WIN32

static SOCKET g_server;

static uint16_t start_server(void)
{
	WSADATA wd;
	WSAStartup(MAKEWORD(2, 2), &wd);
	g_server = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port        = 0;
	bind(g_server, (struct sockaddr *)&addr, sizeof(addr));
	listen(g_server, 1);
	int len = sizeof(addr);
	getsockname(g_server, (struct sockaddr *)&addr, &len);
	return ntohs(addr.sin_port);
}

static void server_send_and_close(const char *data, int n)
{
	SOCKET c = accept(g_server, NULL, NULL);
	int sent = 0;
	while (sent < n) {
		int r = send(c, data + sent, n - sent, 0);
		if (r > 0)
			sent += r;
	}
	closesocket(c);
	closesocket(g_server);
}

#else /* POSIX */

static int g_server;

static uint16_t start_server(void)
{
	g_server = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port        = 0;
	bind(g_server, (struct sockaddr *)&addr, sizeof(addr));
	listen(g_server, 1);
	socklen_t len = sizeof(addr);
	getsockname(g_server, (struct sockaddr *)&addr, &len);
	return ntohs(addr.sin_port);
}

static void server_send_and_close(const char *data, int n)
{
	int c = accept(g_server, NULL, NULL);
	int sent = 0;
	while (sent < n) {
		int r = (int)send(c, data + sent, (size_t)(n - sent), 0);
		if (r > 0)
			sent += r;
	}
	close(c);
	close(g_server);
}

#endif /* _WIN32 */

/* ================================================================
 * 1. test_address_parse
 * ================================================================ */

static void test_address_parse(void)
{
	/* Valid IPv4 */
	{
		NetworkAddressResult r = fun_network_address_parse("127.0.0.1:8080");
		ASSERT_NO_ERROR(r);
		assert(r.value.family == NETWORK_ADDRESS_IPV4);
		assert(r.value.bytes[0] == 127);
		assert(r.value.bytes[1] == 0);
		assert(r.value.bytes[2] == 0);
		assert(r.value.bytes[3] == 1);
		assert(r.value.port == 8080);
	}

	/* Valid IPv6 */
	{
		NetworkAddressResult r = fun_network_address_parse("[::1]:9000");
		ASSERT_NO_ERROR(r);
		assert(r.value.family == NETWORK_ADDRESS_IPV6);
		assert(r.value.port == 9000);
		for (int i = 0; i < 15; i++)
			assert(r.value.bytes[i] == 0);
		assert(r.value.bytes[15] == 1);
	}

	/* Missing port */
	{
		NetworkAddressResult r = fun_network_address_parse("127.0.0.1");
		ASSERT_ERROR(r);
	}

	/* Octet > 255 */
	{
		NetworkAddressResult r = fun_network_address_parse("999.0.0.1:80");
		ASSERT_ERROR(r);
	}

	/* Hostname rejected */
	{
		NetworkAddressResult r = fun_network_address_parse("example.com:80");
		ASSERT_ERROR(r);
	}

	/* Port > 65535 */
	{
		NetworkAddressResult r = fun_network_address_parse("1.2.3.4:65536");
		ASSERT_ERROR(r);
	}

	print_ok("test_address_parse");
}

/* ================================================================
 * 2. test_address_format
 * ================================================================ */

static void test_address_format(void)
{
	/* Format IPv4 */
	{
		NetworkAddressResult parse = fun_network_address_parse("10.0.0.1:3000");
		ASSERT_NO_ERROR(parse);
		char buf[64];
		voidResult fmt = fun_network_address_to_string(parse.value, buf,
													   sizeof(buf));
		ASSERT_NO_ERROR(fmt);
		/* Verify the formatted string contains the expected IP and port */
		int ok = 0;
		/* Simple manual strcmp — no string.h in test allowed? Actually
		 * test files may use standard headers per the instructions.
		 * We use assert with manual check to avoid string.h dependency. */
		const char *expected = "10.0.0.1:3000";
		const char *a = buf;
		const char *b = expected;
		while (*a && *b && *a == *b) { a++; b++; }
		ok = (*a == '\0' && *b == '\0');
		assert(ok);
	}

	/* Buffer too small */
	{
		NetworkAddressResult parse = fun_network_address_parse("127.0.0.1:8080");
		ASSERT_NO_ERROR(parse);
		char tiny[5];
		voidResult fmt = fun_network_address_to_string(parse.value, tiny,
													   sizeof(tiny));
		ASSERT_ERROR(fmt);
	}

	print_ok("test_address_format");
}

/* ================================================================
 * 3. test_connect_fails
 *    Connect to 127.0.0.1:1 — expect ASYNC_ERROR
 * ================================================================ */

static void test_connect_fails(void)
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:1");
	ASSERT_NO_ERROR(ar);

	TcpNetworkConnection conn = (TcpNetworkConnection)0;
	AsyncResult result = fun_network_tcp_connect(ar.value, &conn);
	/* Wait up to 1000ms */
	fun_async_await(&result, 1000);
	assert(result.status == ASYNC_ERROR);

	print_ok("test_connect_fails");
}

/* ================================================================
 * 4. test_tcp_round_trip
 *    Start a loopback server, connect, server sends 8 bytes,
 *    client receive_exact(4) twice.
 * ================================================================ */

static void test_tcp_round_trip(void)
{
	uint16_t port = start_server();

	/* Build connect address */
	char addr_str[32];
	/* Manually build "127.0.0.1:NNNNN" */
	{
		char *p = addr_str;
		const char *prefix = "127.0.0.1:";
		while (*prefix)
			*p++ = *prefix++;
		/* Convert port to decimal */
		char tmp[8];
		int tlen = 0;
		uint16_t pv = port;
		if (pv == 0) {
			tmp[tlen++] = '0';
		} else {
			char rev[8];
			int rlen = 0;
			while (pv > 0) {
				rev[rlen++] = (char)('0' + pv % 10);
				pv /= 10;
			}
			for (int i = rlen - 1; i >= 0; i--)
				tmp[tlen++] = rev[i];
		}
		for (int i = 0; i < tlen; i++)
			*p++ = tmp[i];
		*p = '\0';
	}

	NetworkAddressResult ar = fun_network_address_parse(addr_str);
	ASSERT_NO_ERROR(ar);

	TcpNetworkConnection conn = (TcpNetworkConnection)0;
	AsyncResult cr = fun_network_tcp_connect(ar.value, &conn);
	fun_async_await(&cr, 3000);
	assert(cr.status == ASYNC_COMPLETED);
	assert(conn != (TcpNetworkConnection)0);

	/* Server sends 8 bytes and closes */
	server_send_and_close("ABCDEFGH", 8);

	/* First receive_exact(4): should get "ABCD" */
	char buf1[4];
	NetworkBuffer nb1;
	nb1.data   = buf1;
	nb1.length = 0;
	AsyncResult rr1 = fun_network_tcp_receive_exact(conn, &nb1, 4);
	fun_async_await(&rr1, 3000);
	assert(rr1.status == ASYNC_COMPLETED);
	assert(nb1.length == 4);
	assert(buf1[0] == 'A');
	assert(buf1[1] == 'B');
	assert(buf1[2] == 'C');
	assert(buf1[3] == 'D');

	/* Second receive_exact(4): should get "EFGH" from rx_buf (server closed) */
	char buf2[4];
	NetworkBuffer nb2;
	nb2.data   = buf2;
	nb2.length = 0;
	AsyncResult rr2 = fun_network_tcp_receive_exact(conn, &nb2, 4);
	fun_async_await(&rr2, 3000);
	assert(rr2.status == ASYNC_COMPLETED);
	assert(nb2.length == 4);
	assert(buf2[0] == 'E');
	assert(buf2[1] == 'F');
	assert(buf2[2] == 'G');
	assert(buf2[3] == 'H');

	fun_network_tcp_close(conn);

	print_ok("test_tcp_round_trip");
}

/* ================================================================
 * 5. test_udp_send
 *    Fire-and-forget UDP to 127.0.0.1:12345 — expect ASYNC_COMPLETED
 * ================================================================ */

static void test_udp_send(void)
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:12345");
	ASSERT_NO_ERROR(ar);

	const char *msg = "hello";
	AsyncResult result = fun_network_udp_send(ar.value, msg, 5);
	assert(result.status == ASYNC_COMPLETED);

	print_ok("test_udp_send");
}

/* ================================================================
 * main
 * ================================================================ */

int main(void)
{
	test_address_parse();
	test_address_format();
	test_connect_fails();
	test_tcp_round_trip();
	test_udp_send();

	printf("\nAll tests passed.\n");
	return 0;
}
