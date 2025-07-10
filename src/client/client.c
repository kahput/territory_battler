#include "client.h"

#include "common/arena.h"
#include "common/socket.h"
#include "common/types.h"

#include <netinet/in.h>
#include <string.h>

#include <sys/poll.h>
#include <sys/socket.h>

struct client {
	Socket* socket;

	bool valid_connection;
	NetEvent event;
};

Client* client_create(Arena* arena) {
	Client* client = arena_push_type_zero(arena, Client);
	client->socket = socket_create(arena, AF_INET, SOCK_STREAM);
	client->valid_connection = false;

	return client;
}

bool client_connect(Client* client, const char* address, uint16_t port) {
	client->valid_connection = socket_connect(client->socket, address, port);
	return client->valid_connection;
}

void client_update(Arena* arena, Client* client) {
	if (!client->valid_connection)
		return;

	struct pollfd poll_event = {
		.fd = socket_fd(client->socket),
		.events = POLLIN
	};

	bool pollable = poll(&poll_event, 1, 0);
	if (pollable) {
		int32_t result = 0;
		MessageHeader header = { 0 };

		if ((result = socket_recv(client->socket, &header, sizeof(header), MSG_PEEK)) == 0)
			return;

		uint32_t grid_size = (ntohl(header.rows) * ntohl(header.columns));
		uint32_t player_size = header.player_count * sizeof(PlayerState);
		uint8_t buffer[sizeof(MessageHeader) + grid_size + player_size];

		uint32_t received_bytes = 0;
		while (received_bytes < (grid_size + player_size)) {
			if ((result = socket_recv(client->socket, buffer, sizeof(buffer), 0)) == 0)
				return;
			received_bytes += result;
		}

		client->event = (NetEvent){
			.type = NET_EVENT_RECEIVE,
			.client_id = socket_fd(client->socket),
			.data = arena_push_array(arena, uint8_t, result),
			.size = result
		};
		mempcpy(client->event.data, buffer, result);
	}
}

bool client_poll(Client* client, NetEvent* event) {
	if (client->event.type) {
		*event = client->event;

		client->event = (NetEvent){ 0 };
		return true;
	}

	return false;
}

int32_t client_send(Client* client, void* data, uint32_t size) {
	return socket_send(client->socket, data, size);
}
