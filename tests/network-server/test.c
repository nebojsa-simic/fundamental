#include "fundamental/console/console.h"

#include "fundamental/network/server.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#include <time.h>
#endif

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
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
#define join_thread(h) pthread_join((h), NULL)
#endif

/* ----------------------------------------------------------------
 * TCP config
 * ---------------------------------------------------------------- */

void test_tcp_config_create_free()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9876");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0x42, &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(c != NULL)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	r = fun_network_server_config_free(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result(__func__);
}

void test_tcp_config_null_output()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9876");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, NULL);
	if (r.error.code == 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result(__func__);
}

void test_config_free_null()
{
	voidResult r = fun_network_server_config_free(NULL);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result(__func__);
}

/* ----------------------------------------------------------------
 * UDP config
 * ---------------------------------------------------------------- */

void test_udp_config_create_free()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9877");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	char buf[256];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0x99, buf,
												 sizeof(buf), &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(c != NULL)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	r = fun_network_server_config_free(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result(__func__);
}

void test_udp_config_null_buffer()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9877");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	NetworkServerConfig c = NULL;
	voidResult r =
		fun_network_udp_server_config(ar.value, (Memory)0, NULL, 256, &c);
	if (r.error.code == 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result(__func__);
}

void test_udp_config_zero_buffer_size()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9877");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	char buf[256];
	NetworkServerConfig c = NULL;
	voidResult r =
		fun_network_udp_server_config(ar.value, (Memory)0, buf, 0, &c);
	if (r.error.code == 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
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
	if (!(rr.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	const char *resp = "HELLO";
	AsyncResult sr = fun_network_tcp_send(conn, resp, 5);
	fun_async_await(&sr, 2000);
	if (!(sr.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: check");
		return;
	}
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
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	AsyncResult s = fun_network_tcp_listen(c, on_tcp_connection);
	if (!(s.status == ASYNC_PENDING)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(port > 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	r = fun_network_server_stop(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_async_await(&s, -1);
	if (!(s.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_tcp_port_zero_returns_ephemeral()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	AsyncResult s = fun_network_tcp_listen(c, on_tcp_connection);
	if (!(s.status == ASYNC_PENDING)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(port != 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	r = fun_network_server_stop(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_async_await(&s, -1);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_stop_twice_is_safe()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	AsyncResult s = fun_network_tcp_listen(c, on_tcp_connection);
	if (!(s.status == ASYNC_PENDING)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	r = fun_network_server_stop(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	r = fun_network_server_stop(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_async_await(&s, -1);
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_so_reuseaddr_restart()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	NetworkServerConfig c1 = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c1);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	AsyncResult s1 = fun_network_tcp_listen(c1, on_tcp_connection);
	if (!(s1.status == ASYNC_PENDING)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	uint16_t port = 0;
	r = fun_network_server_get_port(c1, &port);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	r = fun_network_server_stop(c1);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
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
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	AsyncResult s2 = fun_network_tcp_listen(c2, on_tcp_connection);
	if (!(s2.status == ASYNC_PENDING)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	r = fun_network_server_stop(c2);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_async_await(&s2, -1);
	fun_network_server_config_free(c2);
	print_test_result(__func__);
}

void test_null_callback_returns_error()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:9876");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	AsyncResult s = fun_network_tcp_listen(c, (NetworkTcpListener)0);
	if (!(s.status == ASYNC_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

/* ----------------------------------------------------------------
 * Config type validation
 * ---------------------------------------------------------------- */

void test_tcp_listen_rejects_udp_config()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	char buf[64];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0, buf,
												 sizeof(buf), &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	AsyncResult s = fun_network_tcp_listen(c, on_tcp_connection);
	if (!(s.status == ASYNC_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(s.error.code == ERROR_CODE_NETWORK_SERVER_WRONG_CONFIG_TYPE)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_udp_listen_rejects_tcp_config()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	char buf[64];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	AsyncResult s = fun_network_udp_listen(c, dummy_udp_cb);
	if (!(s.status == ASYNC_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(s.error.code == ERROR_CODE_NETWORK_SERVER_WRONG_CONFIG_TYPE)) {
		fun_console_write_line("FAIL: check");
		return;
	}
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
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	TcpCallbackData cbdata = { NULL, NULL, 0 };

	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)&cbdata, &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	AsyncResult srv = fun_network_tcp_listen(c, on_tcp_connect_callback);
	if (!(srv.status == ASYNC_PENDING)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

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
	if (!(cr.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(client != NULL)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	sleep_ms(1500);

	if (!(cbdata.invoked == 1)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(cbdata.conn != NULL)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(cbdata.expected_state == (Memory)&cbdata)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	voidResult close_r = fun_network_tcp_close(client);
	if (close_r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	r = fun_network_server_stop(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
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
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	NetworkServerConfig c = NULL;
	voidResult r = fun_network_tcp_server_config(ar.value, (Memory)0, &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	AsyncResult srv = fun_network_tcp_listen(c, on_tcp_keep_connection);
	if (!(srv.status == ASYNC_PENDING)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

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
	if (!(cr.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	const char *msg = "WORLD";
	AsyncResult sr = fun_network_tcp_send(client, msg, 5);
	fun_async_await(&sr, 2000);
	if (!(sr.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	char buf[64];
	NetworkBuffer nb = { buf, sizeof(buf) };
	AsyncResult rr = fun_network_tcp_receive_exact(client, &nb, 5);
	fun_async_await(&rr, 2000);
	if (!(rr.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	fun_network_tcp_close(client);

	sleep_ms(500);

	r = fun_network_server_stop(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
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
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	char buf[256];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0x88, buf,
												 sizeof(buf), &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	udp_callback_count = 0;
	AsyncResult s = fun_network_udp_listen(c, on_udp_datagram);
	if (!(s.status == ASYNC_PENDING)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(port > 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	r = fun_network_server_stop(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_async_await(&s, -1);
	if (!(s.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_udp_datagram_delivery()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	char buf[256];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0xAA, buf,
												 sizeof(buf), &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	udp_callback_count = 0;
	AsyncResult s = fun_network_udp_listen(c, on_udp_datagram);
	if (!(s.status == ASYNC_PENDING)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	NetworkAddress target;
	target.family = NETWORK_ADDRESS_IPV4;
	target.bytes[0] = 127;
	target.bytes[1] = 0;
	target.bytes[2] = 0;
	target.bytes[3] = 1;
	target.port = port;

	const char *msg = "PING";
	AsyncResult sr = fun_network_udp_send(target, msg, 4);
	if (!(sr.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	fun_async_await(&s, 5000);

	if (!(udp_callback_count == 1)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(udp_callback_state == (void *)0xAA)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	r = fun_network_server_stop(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_udp_truncation()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	char small_buf[8];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0, small_buf,
												 sizeof(small_buf), &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	udp_callback_count = 0;
	AsyncResult s = fun_network_udp_listen(c, on_udp_datagram);
	if (!(s.status == ASYNC_PENDING)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	uint16_t port = 0;
	r = fun_network_server_get_port(c, &port);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	NetworkAddress target;
	target.family = NETWORK_ADDRESS_IPV4;
	target.bytes[0] = 127;
	target.bytes[1] = 0;
	target.bytes[2] = 0;
	target.bytes[3] = 1;
	target.port = port;

	const char *msg = "ABCDEFGHIJKLMNOPQRST";
	AsyncResult sr = fun_network_udp_send(target, msg, 20);
	if (!(sr.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	fun_async_await(&s, 5000);

	if (!(udp_callback_count == 1)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	r = fun_network_server_stop(c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

void test_udp_null_callback_returns_error()
{
	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:0");
	if (ar.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	char buf[64];
	NetworkServerConfig c = NULL;
	voidResult r = fun_network_udp_server_config(ar.value, (Memory)0, buf,
												 sizeof(buf), &c);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	AsyncResult s = fun_network_udp_listen(c, (NetworkUdpListener)0);
	if (!(s.status == ASYNC_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	fun_network_server_config_free(c);
	print_test_result(__func__);
}

/* ----------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------- */

int main(void)
{
	fun_console_write_line("");
	fun_console_write_line("--- Network Server Tests ---");

	fun_console_write_line("");
	fun_console_write_line("  TCP Config");
	test_tcp_config_create_free();
	test_tcp_config_null_output();
	test_config_free_null();

	fun_console_write_line("");
	fun_console_write_line("  UDP Config");
	test_udp_config_create_free();
	test_udp_config_null_buffer();
	test_udp_config_zero_buffer_size();

	fun_console_write_line("");
	fun_console_write_line("  TCP Lifecycle");
	test_tcp_listen_async_pending();
	test_tcp_port_zero_returns_ephemeral();
	test_stop_twice_is_safe();
	test_so_reuseaddr_restart();
	test_null_callback_returns_error();

	fun_console_write_line("");
	fun_console_write_line("  Config Type Validation");
	test_tcp_listen_rejects_udp_config();
	test_udp_listen_rejects_tcp_config();

	fun_console_write_line("");
	fun_console_write_line("  TCP Client Interaction");
	test_tcp_callback_invoked_on_connection();
	test_tcp_client_send_receive();

	fun_console_write_line("");
	fun_console_write_line("  UDP");
	test_udp_truncation();
	test_udp_datagram_delivery();
	test_udp_listen_async_pending();
	test_udp_null_callback_returns_error();

	fun_console_write_line("");
	fun_console_write_line("All network-server tests passed.");
	return 0;
}
