#include "common/arena.h"
#include "common/sockets.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "5050"
#define BACKLOG 10

typedef struct _game_state {
	Arena *permanent_arena, *frame_arena;
} GameState;

int main() {
	GameState state = {
		.permanent_arena = arena_alloc(),
		.frame_arena = arena_alloc()
	};

	Socket* server_socket = socket_create(state.permanent_arena, AF_INET, SOCK_STREAM);
	if (socket_bind(server_socket, NULL, 5050) == false) {
		return -1;
	}
	if (socket_listen(server_socket, BACKLOG) == false) {
		arena_free(state.frame_arena);
		arena_free(state.permanent_arena);

		return -1;
	}

	printf("Server listening on %s...\n", PORT);

	while (1) {
		arena_clear(state.frame_arena);
		Socket* client_socket = socket_accept(state.frame_arena, server_socket);
	}

	arena_free(state.frame_arena);
	arena_free(state.permanent_arena);

	return 0;
}
