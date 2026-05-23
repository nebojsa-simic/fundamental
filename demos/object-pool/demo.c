#include "fundamental/network/server.h"
#include "fundamental/network/network.h"
#include "fundamental/object-pool/object-pool.h"
#include "fundamental/console/console.h"
#include "fundamental/string/string.h"
#include "fundamental/memory/memory.h"

#define MAX_LOCK_NAME 63
#define MAX_HELD_LOCKS 8
#define SESSION_POOL_SIZE 4
#define LOCK_POOL_SIZE 8

typedef struct {
	TcpNetworkConnection conn;
	int held_lock_indices[MAX_HELD_LOCKS];
	int lock_count;
} Session;

DEFINE_OBJECT_POOL_TYPE(Session)

typedef struct {
	char name[MAX_LOCK_NAME + 1];
	int owner_index;
	int in_use;
} LockSlot;

DEFINE_OBJECT_POOL_TYPE(LockSlot)

typedef struct {
	SessionPool session_pool;
	LockSlotPool lock_pool;
	int next_session_id;
} ServerState;

static void send_str(TcpNetworkConnection conn, String s)
{
	AsyncResult sr = fun_network_tcp_send(conn, s, fun_string_length(s));
	fun_async_await(&sr, 500);
}

static void send_line(TcpNetworkConnection conn, String s)
{
	send_str(conn, s);
	char nl = '\n';
	AsyncResult nr = fun_network_tcp_send(conn, &nl, 1);
	fun_async_await(&nr, 500);
}

static void read_line(TcpNetworkConnection conn, char *buf, size_t size)
{
	size_t pos = 0;
	while (pos < size - 1) {
		char ch;
		NetworkBuffer nb = { &ch, 1 };
		AsyncResult rr = fun_network_tcp_receive_exact(conn, &nb, 1);
		fun_async_await(&rr, -1);
		if (rr.status != ASYNC_COMPLETED)
			return;
		if (ch == '\n') {
			buf[pos] = '\0';
			return;
		}
		if (ch != '\r')
			buf[pos++] = ch;
	}
	buf[pos] = '\0';
}

static void handle_client(TcpNetworkConnection conn, ServerState *state,
						  Session *session)
{
	char line[256];
	int session_index =
		((char *)session - (char *)state->session_pool.pool.memory) /
		state->session_pool.pool.elementSize;

	while (1) {
		read_line(conn, line, sizeof(line));
		if (line[0] == '\0')
			break;

		if (fun_string_length(line) >= 7 && line[0] == 'A' && line[1] == 'C' &&
			line[2] == 'Q' && line[3] == 'U' && line[4] == 'I' &&
			line[5] == 'R' && line[6] == 'E' && line[7] == ' ') {
			String lock_name = line + 8;

			LockSlot *l = fun_object_pool_LockSlot_acquire(&state->lock_pool);
			if (l == (LockSlot *)0) {
				send_line(conn, "NOLOCK");
				continue;
			}

			size_t ln = fun_string_length(lock_name);
			if (ln > MAX_LOCK_NAME)
				ln = MAX_LOCK_NAME;
			fun_string_copy(lock_name, l->name, MAX_LOCK_NAME + 1);
			l->owner_index = session_index;
			l->in_use = 1;

			if (session->lock_count < MAX_HELD_LOCKS) {
				size_t li = ((char *)l - (char *)state->lock_pool.pool.memory) /
							state->lock_pool.pool.elementSize;
				session->held_lock_indices[session->lock_count++] = (int)li;
			}

			send_line(conn, "OK");
		} else if (fun_string_length(line) >= 7 && line[0] == 'R' &&
				   line[1] == 'E' && line[2] == 'L' && line[3] == 'E' &&
				   line[4] == 'A' && line[5] == 'S' && line[6] == 'E' &&
				   line[7] == ' ') {
			String lock_name = line + 8;
			int found = 0;

			for (size_t i = 0; i < state->lock_pool.pool.capacity; i++) {
				LockSlot *l =
					(LockSlot *)((char *)state->lock_pool.pool.memory +
								 i * state->lock_pool.pool.elementSize);
				if (l->owner_index == session_index &&
					fun_string_compare(l->name, lock_name) == 0) {
					l->owner_index = -1;
					l->in_use = 0;
					fun_string_copy("", l->name, MAX_LOCK_NAME + 1);
					fun_object_pool_LockSlot_release(&state->lock_pool, l);
					found = 1;
					break;
				}
			}
			send_line(conn, found ? "OK" : "NOLOCK");
		} else if (line[0] == 'L' && line[1] == 'I' && line[2] == 'S' &&
				   line[3] == 'T') {
			for (size_t i = 0; i < state->lock_pool.pool.capacity; i++) {
				LockSlot *l =
					(LockSlot *)((char *)state->lock_pool.pool.memory +
								 i * state->lock_pool.pool.elementSize);
				if (l->in_use) {
					char buf[128];
					fun_string_copy(l->name, buf, sizeof(buf));
					size_t nl = fun_string_length(buf);
					buf[nl] = ' ';
					buf[nl + 1] = ':';
					buf[nl + 2] = ' ';
					fun_string_from_int(l->owner_index, 10, buf + nl + 3,
										sizeof(buf) - nl - 3);
					send_line(conn, buf);
				}
			}
			send_line(conn, "OK");
		} else if (line[0] == 'Q' && line[1] == 'U' && line[2] == 'I' &&
				   line[3] == 'T') {
			send_line(conn, "BYE");
			break;
		}
	}

	// Auto-release all held locks
	for (int i = 0; i < session->lock_count; i++) {
		int idx = session->held_lock_indices[i];
		LockSlot *l = (LockSlot *)((char *)state->lock_pool.pool.memory +
								   idx * state->lock_pool.pool.elementSize);
		if (l->owner_index == session_index) {
			l->owner_index = -1;
			l->in_use = 0;
			fun_string_copy("", l->name, MAX_LOCK_NAME + 1);
			fun_object_pool_LockSlot_release(&state->lock_pool, l);
		}
	}

	session->lock_count = 0;
	fun_object_pool_Session_release(&state->session_pool, session);
	fun_network_tcp_close(conn);
}

static void on_connected(TcpNetworkConnection conn, Memory state_ptr)
{
	ServerState *state = (ServerState *)state_ptr;

	Session *session = fun_object_pool_Session_acquire(&state->session_pool);
	if (session == (Session *)0) {
		send_line(conn, "BUSY");
		fun_network_tcp_close(conn);
		return;
	}

	session->conn = conn;
	session->lock_count = 0;

	char buf[64];
	fun_string_copy("connected #", buf, sizeof(buf));
	size_t nl = fun_string_length(buf);
	fun_string_from_int(state->next_session_id, 10, buf + nl, sizeof(buf) - nl);
	fun_console_write_line(buf);

	handle_client(conn, state, session);

	fun_string_copy("disconnected #", buf, sizeof(buf));
	nl = fun_string_length(buf);
	fun_string_from_int(state->next_session_id, 10, buf + nl, sizeof(buf) - nl);
	fun_console_write_line(buf);

	state->next_session_id++;
}

int main(void)
{
	ServerState state;
	state.next_session_id = 1;

	ObjectPoolResult spr = fun_object_pool_Session_create(SESSION_POOL_SIZE);
	if (fun_error_is_error(spr.error)) {
		fun_console_error_line("failed to create session pool");
		return 1;
	}
	state.session_pool.pool = spr.value;

	ObjectPoolResult lpr = fun_object_pool_LockSlot_create(LOCK_POOL_SIZE);
	if (fun_error_is_error(lpr.error)) {
		fun_console_error_line("failed to create lock pool");
		return 1;
	}
	state.lock_pool.pool = lpr.value;

	NetworkAddressResult ar = fun_network_address_parse("127.0.0.1:8080");
	if (fun_error_is_error(ar.error)) {
		fun_console_error_line("bad address");
		return 1;
	}

	NetworkServerConfig config = (NetworkServerConfig)0;
	voidResult cr =
		fun_network_tcp_server_config(ar.value, (Memory)&state, &config);
	if (fun_error_is_error(cr.error)) {
		fun_console_error_line("config failed");
		return 1;
	}

	fun_console_write_line("lockd on 127.0.0.1:8080");

	AsyncResult server = fun_network_tcp_listen(config, on_connected);
	if (server.status == ASYNC_ERROR) {
		fun_console_write_line("listen failed");
		fun_network_server_config_free(config);
		return 1;
	}

	fun_async_await(&server, -1);
	fun_network_server_config_free(config);
	fun_object_pool_LockSlot_destroy(&state.lock_pool);
	fun_object_pool_Session_destroy(&state.session_pool);
	fun_console_write_line("stopped");
	return 0;
}
