#include "server.h"
#include "common/arena.h"
#include "common/socket.h"
#include "common/types.h"

#include <sys/poll.h>
#include <sys/socket.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct client_slot {
	Socket* sock;
} ClientSlot;

struct server {
	Socket* sock;
	Arena* arena;

	ClientSlot* clients;
	int32_t client_count, client_capacity;

	NetEvent* event_stack;
	int32_t event_count, event_capacity;
};

Server* server_create(Arena* arena, uint16_t port, int32_t max_clients) {
	Socket* server_socket = socket_create(arena, AF_INET, SOCK_STREAM);
	if (socket_bind(server_socket, NULL, port) == false)
		return NULL;

	if (socket_listen(server_socket, 5) == false)
		return NULL;

	Server* server = arena_push_type_zero(arena, Server);
	server->arena = arena;

	server->sock = server_socket;
	server->clients = arena_push_array(arena, ClientSlot, max_clients);

	return server;
}

void server_update(Arena* arena, Server* server) {
	uint32_t poll_list_count = server->client_count + 1;
	struct pollfd poll_list[poll_list_count];

	poll_list[0] = (struct pollfd){
		.fd = socket_fd(server->sock),
		.events = POLLIN
	};
	for (uint32_t poll_index = 1, client_index = 0; poll_index < poll_list_count; poll_index++, client_index++) {
		poll_list[poll_index] = (struct pollfd){
			.fd = socket_fd(server->clients[client_index].sock),
			.events = POLLIN
		};
	}

	int32_t poll_count = poll(poll_list, poll_list_count, -1);

	if (poll_count <= 0) {
		perror("poll");
		return;
	}

	server->event_stack = arena_push_array(arena, NetEvent, poll_count);
	server->event_count = 0;
	server->event_capacity = poll_count;

	for (uint32_t poll_index = 0; poll_index < poll_list_count; poll_index++) {
		if (poll_list[poll_index].revents == 0)
			continue;
		// New connection
		if (poll_list[poll_index].fd == socket_fd(server->sock)) {
			if ((poll_list[poll_index].revents & POLLIN) == true) {
				server->event_stack[server->event_count++] = (NetEvent){
					.type = NET_EVENT_CONNECT,
					.client_id = server->client_count
				};

				server->clients[server->client_count++] = (ClientSlot){
					.sock = socket_accept(server->arena, server->sock)
				};
			}
		}
		// Data from client
		else {
			int32_t client_index = poll_index - 1;
			uint8_t buffer[1024];
			int32_t result = 0;
			result = socket_recv(server->clients[client_index].sock, buffer, sizeof(buffer), 0);

			// Disconnect message
			if (result == 0) {
				server->event_stack[server->event_count++] = (NetEvent){
					.type = NET_EVENT_DISCONNECT,
					.client_id = client_index
				};
				close(socket_fd(server->clients[client_index].sock));
				server->clients[client_index] = (ClientSlot){ .sock = server->clients[server->client_count - 1].sock };

				server->clients[server->client_count - 1].sock = NULL;
				server->client_count--;
			}

			// Data message
			else {
				server->event_stack[server->event_count] = (NetEvent){
					.type = NET_EVENT_RECEIVE,
					.client_id = client_index,
					.data = arena_push_array(arena, uint8_t, result),
					.size = result
				};
				memcpy(server->event_stack[server->event_count].data, buffer, result);
				server->event_count++;
			}
		}
	}
}

bool server_poll(Server* server, NetEvent* event) {
	if (server->event_count) {
		(*event) = (NetEvent)server->event_stack[--server->event_count];
		return true;
	}
	return false;
}

void server_send(Server* server, int32_t client_id, void* data, int32_t size) {
	printf("Sending message [%s] to client %i\n", (char*)data, client_id + 1);
	socket_send(server->clients[client_id].sock, data, size);
}
void server_broadcast(Server* server, void* data, int32_t size) {
	for (uint32_t i = 0; i < server->client_count; i++) {
		printf("Sending message to client %i\n", i + 1);
		socket_send(server->clients[i].sock, data, size);
	}
}
