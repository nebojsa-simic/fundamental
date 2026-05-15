#include <assert.h>
#include <stdio.h>

#include "fundamental/network/server.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#include <time.h>
#endif

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

#ifdef _WIN32
static void sleep_ms(int ms)
{
	Sleep(ms);
}
#else
static void sleep_ms(int ms)
{
	struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
	nanosleep(&ts, NULL);
}
#endif

/* Threading helpers for running server in background */
typedef struct {
	AsyncResult *server;
	volatile int running;
} ServerThreadData;

#ifdef _WIN32
static DWORD WINAPI server_thread(LPVOID param)
#else
static void *server_thread(void *param)
#endif
{
	ServerThreadData *d = (ServerThreadData *)param;
	fun_async_await(d->server, -1);
	d->running = 0;
#ifdef _WIN32
	return 0;
#else
	return NULL;
#endif
}

#ifdef _WIN32
typedef HANDLE thread_h;
#define create_thread(h, d) \
	(*(h) = CreateThread(NULL, 0, server_thread, (d), 0, NULL))
#define join_thread(h)              \
	WaitForSingleObject((h), 5000); \
	CloseHandle(h)
#else
typedef pthread_t thread_h;
#define create_thread(h, d) pthread_create((h), NULL, server_thread, (d))
#define join_thread(h) pthread_join(*(h), NULL)
#endif

/* ----------------------------------------------------------------
 * TCP config
 * ---------------------------------------------------------------- */

void test_tcp_config_create_free()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9876");
	ASSERT_NO_ERROR(ar);
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0x42, &c);
	ASSERT_NO_ERROR(r);
	assert(c != NULL);
	r = fun_network_server_config_free(c);
	ASSERT_NO_ERROR(r);
	print_test_result(__func__);
}

void test_tcp_config_null_output()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9876");
	ASSERT_NO_ERROR(ar);
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, NULL);
	ASSERT_ERROR(r);
	print_test_result(__func__);
}

void test_config_free_null()
{
	voidResult r = fun_network_server_config_free(NULL);
	ASSERT_NO_ERROR(r);
	print_test_result(__func__);
}

/* ----------------------------------------------------------------
 * UDP config
 * ---------------------------------------------------------------- */

void test_udp_config_create_free()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9877");
	ASSERT_NO_ERROR(ar);
	char buf[256];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0x99, buf,
												 sizeof(buf), &c);
	ASSERT_NO_ERROR(r);
	assert(c != NULL);
	r = fun_network_server_config_free(c);
	ASSERT_NO_ERROR(r);
	print_test_result(__func__);
}

void test_udp_config_null_buffer()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9877");
	ASSERT_NO_ERROR(ar);
	NetworkServerConfig c = NULL;
	voidResult r =
		fun_network_udp_server_config(ar.value, (Memory)0, NULL, 256, &c);
	ASSERT_ERROR(r);
	print_test_result(__func__);
}

void test_udp_config_zero_buffer_size()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9877");
	ASSERT_NO_ERROR(ar);
	char buf[256];
	NetworkServerConfig c = NULL;
	voidResult r =
		fun_network_udp_server_config(ar.value, (Memory)0, buf, 0, &c);
	ASSERT_ERROR(r);
	print_test_result(__func__);
}

/* ----------------------------------------------------------------
 * Callbacks
 * ---------------------------------------------------------------- */

static int tcp_callback_count = 0;
static void *tcp_callback_state = NULL;

static void on_tcp_connection(TcpNetworkConnection conn, Memory state)
{
	tcp_callback_count++;
	tcp_callback_state = (void *)state;
	fun_network_tcp_close(conn);
}

static TcpNetworkConnection g_server_conn = NULL;

static void on_tcp_keep_connection(TcpNetworkConnection conn, Memory state)
{
	(void)state;
	g_server_conn = conn;
	char buf[64];
	NetworkBuffer nb = { buf, sizeof(buf) };
	AsyncResult rr = fun_network_tcp_receive_exact(conn, &nb, 5);
	fun_async_await(&rr, 2000);
	assert(rr.status == ASYNC_COMPLETED);
	const char *resp = "HELLO";
	AsyncResult sr = fun_network_tcp_send(conn, resp, 5);
	fun_async_await(&sr, 2000);
	assert(sr.status == ASYNC_COMPLETED);
}

static int udp_callback_count = 0;
static void *udp_callback_state = NULL;

static void on_udp_datagram(NetworkAddress source, NetworkBuffer buffer,
							Memory state)
{
	udp_callback_count++;
	udp_callback_state = (void *)state;
	(void)source;
	(void)buffer;
}

static void dummy_udp_cb(NetworkAddress src, NetworkBuffer buf, Memory st)
{
	(void)src;
	(void)buf;
	(void)st;
}

/* ----------------------------------------------------------------
 * TCP lifecycle
 * ---------------------------------------------------------------- */

void test_tcp_listen_async_pending()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	ASSERT_NO_ERROR(r);
	AsyncResult s = fun_network_tcp_listen(c, on_tcp_connection);
	assert(s.status == ASYNC_PENDING);
	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	ASSERT_NO_ERROR(r);
	assert(port > 0);
	r = fun_network_server_stop(c);
	ASSERT_NO_ERROR(r);
	fun_async_await(&s, -1);
	assert(s.status == ASYNC_COMPLETED);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_tcp_port_zero_returns_ephemeral()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	ASSERT_NO_ERROR(r);
	AsyncResult s = fun_network_tcp_listen(c, on_tcp_connection);
	assert(s.status == ASYNC_PENDING);
	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	ASSERT_NO_ERROR(r);
	assert(port != 0);
	r = fun_network_server_stop(c);
	ASSERT_NO_ERROR(r);
	fun_async_await(&s, -1);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_stop_twice_is_safe()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	ASSERT_NO_ERROR(r);
	AsyncResult s = fun_network_tcp_listen(c, on_tcp_connection);
	assert(s.status == ASYNC_PENDING);
	r = fun_network_server_stop(c);
	ASSERT_NO_ERROR(r);
	r = fun_network_server_stop(c);
	ASSERT_NO_ERROR(r);
	fun_async_await(&s, -1);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_so_reuseaddr_restart()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);

	NetworkServerConfig c1 = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c1);
	ASSERT_NO_ERROR(r);
	AsyncResult s1 = fun_network_tcp_listen(c1, on_tcp_connection);
	assert(s1.status == ASYNC_PENDING);

	uint16_t port = 0;
	r = fun_network_server_get_port(c1, &port);
	ASSERT_NO_ERROR(r);

	r = fun_network_server_stop(c1);
	ASSERT_NO_ERROR(r);
	fun_async_await(&s1, -1);
	fun_network_server_config_free(c1);

	NetworkAddress addr2;
	addr2.family = NETWORK_ADDRESS_IPV4;
	addr2.bytes[0] = 127;
	addr2.bytes[1] = 0;
	addr2.bytes[2] = 0;
	addr2.bytes[3] = 1;
	addr2.port = port;

	NetworkServerConfig c2 = NULL;
	r = fun_network_tcp_server_config(addr2, (Memory)0, &c2);
	ASSERT_NO_ERROR(r);
	AsyncResult s2 = fun_network_tcp_listen(c2, on_tcp_connection);
	assert(s2.status == ASYNC_PENDING);

	r = fun_network_server_stop(c2);
	ASSERT_NO_ERROR(r);
	fun_async_await(&s2, -1);
	fun_network_server_config_free(c2);
	print_test_result(__func__);
}

void test_null_callback_returns_error()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9876");
	ASSERT_NO_ERROR(ar);
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	ASSERT_NO_ERROR(r);
	AsyncResult s = fun_network_tcp_listen(c, (NetworkTcpListener)0);
	assert(s.status == ASYNC_ERROR);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

/* ----------------------------------------------------------------
 * Config type validation
 * ---------------------------------------------------------------- */

void test_tcp_listen_rejects_udp_config()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);
	char buf[64];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0, buf,
												 sizeof(buf), &c);
	ASSERT_NO_ERROR(r);
	AsyncResult s = fun_network_tcp_listen(c, on_tcp_connection);
	assert(s.status == ASYNC_ERROR);
	assert(s.error.code == ERROR_CODE_NETWORK_SERVER_WRONG_CONFIG_TYPE);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_udp_listen_rejects_tcp_config()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);
	char buf[64];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	ASSERT_NO_ERROR(r);
	AsyncResult s = fun_network_udp_listen(c, dummy_udp_cb);
	assert(s.status == ASYNC_ERROR);
	assert(s.error.code == ERROR_CODE_NETWORK_SERVER_WRONG_CONFIG_TYPE);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

/* ----------------------------------------------------------------
 * TCP client connection + callback
 * ---------------------------------------------------------------- */

typedef struct {
	TcpNetworkConnection conn;
	Memory expected_state;
	int invoked;
} TcpCallbackData;

static void on_tcp_connect_callback(TcpNetworkConnection conn, Memory state)
{
	TcpCallbackData *d = (TcpCallbackData *)state;
	d->conn = conn;
	d->invoked = 1;
	d->expected_state = state;
	fun_network_tcp_close(conn);
}

void test_tcp_callback_invoked_on_connection()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);

	TcpCallbackData cbdata = { NULL, NULL, 0 };

	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)&cbdata, &c);
	ASSERT_NO_ERROR(r);

	AsyncResult srv = fun_network_tcp_listen(c, on_tcp_connect_callback);
	assert(srv.status == ASYNC_PENDING);

	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	ASSERT_NO_ERROR(r);

	ServerThreadData std = { &srv, 1 };
	thread_h th;
	create_thread(&th, &std);
	sleep_ms(100);

	NetworkAddress target;
	target.family = NETWORK_ADDRESS_IPV4;
	target.bytes[0] = 127;
	target.bytes[1] = 0;
	target.bytes[2] = 0;
	target.bytes[3] = 1;
	target.port = port;

	TcpNetworkConnection client = NULL;
	AsyncResult cr = fun_network_tcp_connect(target, &client);
	fun_async_await(&cr, 2000);
	assert(cr.status == ASYNC_COMPLETED);
	assert(client != NULL);

	sleep_ms(1500);

	assert(cbdata.invoked == 1);
	assert(cbdata.conn != NULL);
	assert(cbdata.expected_state == (Memory)&cbdata);

	voidResult close_r = fun_network_tcp_close(client);
	ASSERT_NO_ERROR(close_r);

	r = fun_network_server_stop(c);
	ASSERT_NO_ERROR(r);
	join_thread(th);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

/* ----------------------------------------------------------------
 * TCP client send/receive
 * ---------------------------------------------------------------- */

void test_tcp_client_send_receive()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);

	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	ASSERT_NO_ERROR(r);

	AsyncResult srv = fun_network_tcp_listen(c, on_tcp_keep_connection);
	assert(srv.status == ASYNC_PENDING);

	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	ASSERT_NO_ERROR(r);

	ServerThreadData std = { &srv, 1 };
	thread_h th;
	create_thread(&th, &std);
	sleep_ms(100);

	NetworkAddress target;
	target.family = NETWORK_ADDRESS_IPV4;
	target.bytes[0] = 127;
	target.bytes[1] = 0;
	target.bytes[2] = 0;
	target.bytes[3] = 1;
	target.port = port;

	TcpNetworkConnection client = NULL;
	AsyncResult cr = fun_network_tcp_connect(target, &client);
	fun_async_await(&cr, 2000);
	assert(cr.status == ASYNC_COMPLETED);

	const char *msg = "WORLD";
	AsyncResult sr = fun_network_tcp_send(client, msg, 5);
	fun_async_await(&sr, 2000);
	assert(sr.status == ASYNC_COMPLETED);

	char buf[64];
	NetworkBuffer nb = { buf, sizeof(buf) };
	AsyncResult rr = fun_network_tcp_receive_exact(client, &nb, 5);
	fun_async_await(&rr, 2000);
	assert(rr.status == ASYNC_COMPLETED);

	fun_network_tcp_close(client);

	sleep_ms(500);

	r = fun_network_server_stop(c);
	ASSERT_NO_ERROR(r);
	join_thread(th);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

/* ----------------------------------------------------------------
 * UDP server
 * ---------------------------------------------------------------- */

void test_udp_listen_async_pending()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);
	char buf[256];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0x88, buf,
												 sizeof(buf), &c);
	ASSERT_NO_ERROR(r);
	udp_callback_count = 0;
	AsyncResult s = fun_network_udp_listen(c, on_udp_datagram);
	assert(s.status == ASYNC_PENDING);
	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	ASSERT_NO_ERROR(r);
	assert(port > 0);
	r = fun_network_server_stop(c);
	ASSERT_NO_ERROR(r);
	fun_async_await(&s, -1);
	assert(s.status == ASYNC_COMPLETED);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_udp_datagram_delivery()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);
	char buf[256];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0xAA, buf,
												 sizeof(buf), &c);
	ASSERT_NO_ERROR(r);

	udp_callback_count = 0;
	AsyncResult s = fun_network_udp_listen(c, on_udp_datagram);
	assert(s.status == ASYNC_PENDING);

	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	ASSERT_NO_ERROR(r);

	NetworkAddress target;
	target.family = NETWORK_ADDRESS_IPV4;
	target.bytes[0] = 127;
	target.bytes[1] = 0;
	target.bytes[2] = 0;
	target.bytes[3] = 1;
	target.port = port;

	const char *msg = "PING";
	AsyncResult sr = fun_network_udp_send(target, msg, 4);
	assert(sr.status == ASYNC_COMPLETED);

	/* Poll server — datagram is already buffered, will be picked up */
	fun_async_await(&s, 5000);

	assert(udp_callback_count == 1);
	assert(udp_callback_state == (void *)0xAA);

	r = fun_network_server_stop(c);
	ASSERT_NO_ERROR(r);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_udp_truncation()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);

	char small_buf[8];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0, small_buf,
												 sizeof(small_buf), &c);
	ASSERT_NO_ERROR(r);

	udp_callback_count = 0;
	AsyncResult s = fun_network_udp_listen(c, on_udp_datagram);
	assert(s.status == ASYNC_PENDING);

	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	ASSERT_NO_ERROR(r);

	NetworkAddress target;
	target.family = NETWORK_ADDRESS_IPV4;
	target.bytes[0] = 127;
	target.bytes[1] = 0;
	target.bytes[2] = 0;
	target.bytes[3] = 1;
	target.port = port;

	const char *msg = "ABCDEFGHIJKLMNOPQRST";
	AsyncResult sr = fun_network_udp_send(target, msg, 20);
	assert(sr.status == ASYNC_COMPLETED);

	fun_async_await(&s, 5000);

	assert(udp_callback_count == 1);

	r = fun_network_server_stop(c);
	ASSERT_NO_ERROR(r);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_udp_null_callback_returns_error()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	ASSERT_NO_ERROR(ar);
	char buf[64];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0, buf,
												 sizeof(buf), &c);
	ASSERT_NO_ERROR(r);
	AsyncResult s = fun_network_udp_listen(c, (NetworkUdpListener)0);
	assert(s.status == ASYNC_ERROR);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

/* ----------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------- */

int main(void)
{
	printf("\n--- Network Server Tests ---\n");

	printf("\n  TCP Config\n");
	test_tcp_config_create_free();
	test_tcp_config_null_output();
	test_config_free_null();

	printf("\n  UDP Config\n");
	test_udp_config_create_free();
	test_udp_config_null_buffer();
	test_udp_config_zero_buffer_size();

	printf("\n  TCP Lifecycle\n");
	test_tcp_listen_async_pending();
	test_tcp_port_zero_returns_ephemeral();
	test_stop_twice_is_safe();
	test_so_reuseaddr_restart();
	test_null_callback_returns_error();

	printf("\n  Config Type Validation\n");
	test_tcp_listen_rejects_udp_config();
	test_udp_listen_rejects_tcp_config();

	printf("\n  TCP Client Interaction\n");
	test_tcp_callback_invoked_on_connection();
	test_tcp_client_send_receive();

	printf("\n  UDP\n");
	test_udp_truncation();
	test_udp_datagram_delivery();
	test_udp_listen_async_pending();
	test_udp_null_callback_returns_error();

	printf("\nAll network-server tests passed.\n");
	return 0;
}
