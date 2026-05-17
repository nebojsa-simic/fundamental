#include "fundamental/network/server.h"
#include "fundamental/network/network.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include "fundamental/console/console.h"

#define MAX_TOPICS 32
#define TOPIC_NAME_MAX 32
#define MESSAGE_MAX 512

typedef struct {
	char name[TOPIC_NAME_MAX];
	char last_message[MESSAGE_MAX];
	int has_message;
} Topic;

typedef struct {
	Topic topics[MAX_TOPICS];
	int topic_count;
} BrokerState;

static Topic *find_or_create_topic(BrokerState *bs, String name)
{
	for (int i = 0; i < bs->topic_count; i++) {
		if (fun_string_compare(bs->topics[i].name, name) == 0)
			return &bs->topics[i];
	}
	if (bs->topic_count >= MAX_TOPICS)
		return (Topic *)0;
	Topic *t = &bs->topics[bs->topic_count++];
	fun_string_copy(name, t->name, sizeof(t->name));
	t->has_message = 0;
	return t;
}

static void read_line(TcpNetworkConnection conn, char *buf, size_t size,
					  int *out_len)
{
	char ch;
	NetworkBuffer byte_buf = { &ch, 1 };
	*out_len = 0;

	while (*out_len < (int)size - 1) {
		AsyncResult rr = fun_network_tcp_receive_exact(conn, &byte_buf, 1);
		fun_async_await(&rr, -1);
		if (rr.status != ASYNC_COMPLETED) {
			*out_len = -1;
			return;
		}
		if (ch == '\n') {
			buf[*out_len] = '\0';
			return;
		}
		if (ch != '\r')
			buf[(*out_len)++] = ch;
	}

	buf[*out_len] = '\0';
}

static void send_str(TcpNetworkConnection conn, String s)
{
	AsyncResult sr = fun_network_tcp_send(conn, s, fun_string_length(s));
	fun_async_await(&sr, 500);
}

static void send_ok(TcpNetworkConnection conn)
{
	send_str(conn, "OK\n");
}

static void send_bye(TcpNetworkConnection conn)
{
	send_str(conn, "BYE\n");
}

static void handle_client(TcpNetworkConnection conn, BrokerState *bs)
{
	char line[MESSAGE_MAX + TOPIC_NAME_MAX + 8];
	int len = 0;

	while (1) {
		read_line(conn, line, sizeof(line), &len);
		if (len < 0)
			break;

		if (len == 0)
			continue;

		if (fun_string_length(line) >= 4 && line[0] == 'P' && line[1] == 'U' &&
			line[2] == 'B' && line[3] == ' ') {
			String topic_start = line + 4;
			StringPosition sp = fun_string_index_of(topic_start, " ", 0);
			if (sp >= 0) {
				char topic_name[TOPIC_NAME_MAX];
				fun_string_substring(topic_start, 0, (size_t)sp, topic_name,
									 sizeof(topic_name));
				String message = topic_start + sp + 1;
				Topic *t = find_or_create_topic(bs, topic_name);
				if (t) {
					fun_string_copy(message, t->last_message,
									sizeof(t->last_message));
					t->has_message = 1;
					send_ok(conn);
				}
			}
		} else if (fun_string_length(line) >= 4 && line[0] == 'S' &&
				   line[1] == 'U' && line[2] == 'B' && line[3] == ' ') {
			String topic_name = line + 4;
			Topic *t = find_or_create_topic(bs, topic_name);
			if (t && t->has_message) {
				send_str(conn, t->last_message);
				send_str(conn, "\n");
			}
			send_ok(conn);
		} else if (fun_string_length(line) >= 4 && line[0] == 'Q' &&
				   line[1] == 'U' && line[2] == 'I' && line[3] == 'T') {
			send_bye(conn);
			break;
		}
	}

	fun_network_tcp_close(conn);
}

static void on_client_connected(TcpNetworkConnection conn, Memory state)
{
	BrokerState *bs = (BrokerState *)state;
	fun_console_write_line("client connected");
	handle_client(conn, bs);
	fun_console_write_line("client disconnected");
}

int main(void)
{
	BrokerState bs;
	bs.topic_count = 0;

	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:8080");
	if (fun_error_is_error(ar.error)) {
		fun_console_write_line("bad address");
		return 1;
	}

	NetworkServerConfig config = (NetworkServerConfig)0;
	voidResult cr =
		fun_network_tcp_server_config(ar.value, (Memory)&bs, &config);
	if (fun_error_is_error(cr.error)) {
		fun_console_write_line("config failed");
		return 1;
	}

	AsyncResult server = fun_network_tcp_listen(config, on_client_connected);
	if (server.status == ASYNC_ERROR) {
		fun_console_write_line("listen failed");
		fun_network_server_config_free(config);
		return 1;
	}

	uint16_t port = 0;
	voidResult pr = fun_network_server_get_port(config, &port);
	if (fun_error_is_ok(pr.error)) {
		char port_str[8];
		fun_string_from_int(port, 10, port_str, sizeof(port_str));
		fun_console_write("broker on 127.0.0.1:");
		fun_console_write_line(port_str);
	} else {
		fun_console_write_line("broker on 127.0.0.1:8080");
	}

	fun_async_await(&server, -1);

	fun_network_server_config_free(config);
	fun_console_write_line("stopped");
	return 0;
}
