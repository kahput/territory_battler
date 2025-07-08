#include "common/arena.h"
#include "common/socket.h"
#include "common/types.h"
#include "server/server.h"

// Networking
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <raylib.h>
#include <raymath.h>

#define PORT "5050"
#define BACKLOG 10

#define GRID_SIZE 32
#define ROWS 50
#define COLUMNS 50

typedef struct grid_coordinate {
	int32_t x, y;
} GridCoord;

typedef struct _player {
	uint8_t client_id;

	GridCoord position;
	uint8_t direction, speed, color_id;
} Player;

typedef struct _game_state {
	Arena *permanent_arena, *frame_arena;

	Player *players;
	uint32_t player_count, player_capacity;
} GameState;

// static Color colors[5] = { RAYWHITE, RED, BLUE, GREEN, ORANGE };

uint32_t owned_neighbor_count(uint32_t target, uint32_t index);
bool encolsure_fill(Arena *arena, uint32_t index, uint32_t target, uint8_t grid[ROWS * COLUMNS]);

GridCoord to_2D(int32_t direction);
void print_player(Player p);

int main() {
	GameState state = {
		.permanent_arena = arena_alloc(),
		.frame_arena = arena_alloc(),
		.player_capacity = 4,
	};
	state.players = arena_push_array_zero(state.permanent_arena, Player, state.player_capacity);

	Server *server = server_create(state.permanent_arena, 5050, 4);
	uint8_t *world_grid = arena_push_array_zero(state.permanent_arena, uint8_t, ROWS *COLUMNS);

	while (1) {
		arena_clear(state.frame_arena);
		server_update(state.frame_arena, server);

		NetEvent event;
		while (server_poll(server, &event)) {
			if (event.type == NET_EVENT_CONNECT) {
				printf("New connection\n");

				Player new_player = {
					.client_id = event.client_id,
					.color_id = event.client_id,
					.position = { 3, 3 },
					.speed = 128,
				};
				state.players[state.player_count++] = new_player;

				print_player(new_player);
				char message[1024];
				snprintf(message, sizeof message, "Client %i, your world position is %i, %i", event.client_id + 1, new_player.position.x, new_player.position.y);
				server_broadcast(server, message, sizeof message);
			}
			if (event.type == NET_EVENT_DISCONNECT) {
				state.players[event.client_id] = state.players[state.player_count - 1];
				--state.player_count;

				printf("Client %i connection closed\n", event.client_id + 1);
			}
			if (event.type == NET_EVENT_RECEIVE) {
				uint8_t new_direction = *event.data;
				if (new_direction != 0 && new_direction <= 4)
					state.players[event.client_id].direction = new_direction;
			}
		}

		// WORLD LOGIC

		for (uint32_t i = 0; i < state.player_count; i++) {
			Player *player = &state.players[i];
			if (player->direction != 0) {
				// Vector2 new_position = Vector2Add(player.position, Vector2Scale(to_vec2(player.direction), GRID_SIZE));
				GridCoord direction = to_2D(player->direction);
				GridCoord new_position = { player->position.x + direction.x, player->position.y + direction.y };

				if (new_position.x >= 0 && (int)(new_position.x / GRID_SIZE) < COLUMNS)
					player->position.x = new_position.x;
				if (new_position.y >= 0 && (int)(new_position.y / GRID_SIZE) < ROWS)
					player->position.y = new_position.y;
			}

			uint32_t index = player->position.x + player->position.y * COLUMNS;
			world_grid[index] = player->color_id;
			encolsure_fill(state.frame_arena, index, player->color_id, world_grid);
		}
	}

	arena_free(state.frame_arena);
	arena_free(state.permanent_arena);

	return 0;
}

GridCoord to_2D(int32_t direction) {
	return (GridCoord){
		.x = direction == 2 ? 1 : direction == 4 ? -1
												 : 0,
		.y = direction == 1 ? -1 : direction == 3 ? 1
												  : 0,
	};
}

void print_player(Player p) {
	printf("Player %i {\n\tindex = %i\n\tposition { x = %.2f, y = %.2f }\n}\n",
		p.client_id + 1, p.client_id,
		(float)p.position.x * COLUMNS, (float)p.position.y * ROWS);
}

void flood_fill(Arena *arena, uint32_t index, uint32_t target, uint8_t grid[ROWS * COLUMNS]) {
	int32_t *stack = arena_push_array(arena, int32_t, ROWS *COLUMNS);
	bool *visisted = arena_push_array(arena, bool, ROWS *COLUMNS);
	uint32_t *result = arena_push_array_zero(arena, uint32_t, ROWS *COLUMNS);
	uint32_t stack_pointer = 0, result_pointer = 0;
	stack[stack_pointer++] = index;

	while (stack_pointer) {
		int32_t current_index = stack[--stack_pointer];
		int32_t x = current_index % COLUMNS, y = current_index / COLUMNS;

		if (x < 0 || x >= COLUMNS || y < 0 || y >= ROWS)
			return;

		if (visisted[current_index] == true || grid[current_index] == target)
			continue;

		visisted[current_index] = true;
		result[result_pointer++] = current_index;
		stack[stack_pointer++] = (x + 0) + (y - 1) * COLUMNS;
		stack[stack_pointer++] = (x + 1) + (y - 1) * COLUMNS;
		stack[stack_pointer++] = (x + 1) + (y + 0) * COLUMNS;
		stack[stack_pointer++] = (x + 1) + (y + 1) * COLUMNS;
		stack[stack_pointer++] = (x + 0) + (y + 1) * COLUMNS;
		stack[stack_pointer++] = (x - 1) + (y + 1) * COLUMNS;
		stack[stack_pointer++] = (x - 1) + (y - 0) * COLUMNS;
		stack[stack_pointer++] = (x - 1) + (y - 1) * COLUMNS;
	}

	for (uint32_t i = 0; i < result_pointer; i++) {
		uint32_t index = result[i];
		grid[index] = target;
	}
}

bool encolsure_fill(Arena *arena, uint32_t index, uint32_t target, uint8_t grid[ROWS * COLUMNS]) {
	int32_t *neighbors = arena_push_array(arena, int32_t, 8);
	uint32_t neighbor_count = 0;

	uint32_t x = index % COLUMNS, y = index / COLUMNS;
	if (y > 0 && grid[x + (y - 1) * COLUMNS] != target) {
		neighbors[neighbor_count++] = (x + 0) + (y - 1) * COLUMNS;
	}
	if (y > 0 && x < (COLUMNS - 1) && grid[(x + 1) + (y - 1) * COLUMNS] != target) {
		neighbors[neighbor_count++] = (x + 1) + (y - 1) * COLUMNS;
	}
	if (x < (COLUMNS - 1) && grid[(x + 1) + (y - 0) * COLUMNS] != target) {
		neighbors[neighbor_count++] = (x + 1) + (y - 0) * COLUMNS;
	}
	if (x < (COLUMNS - 1) && y < (ROWS - 1) && grid[(x + 1) + (y + 1) * COLUMNS] != target) {
		neighbors[neighbor_count++] = (x + 1) + (y + 1) * COLUMNS;
	}
	if (y < (ROWS - 1) && grid[(x + 0) + (y + 1) * COLUMNS] != target) {
		neighbors[neighbor_count++] = (x + 0) + (y + 1) * COLUMNS;
	}
	if (y < (ROWS - 1) && x > 0 && grid[(x - 1) + (y + 1) * COLUMNS] != target) {
		neighbors[neighbor_count++] = (x - 1) + (y + 1) * COLUMNS;
	}
	if (x > 0 && grid[(x - 1) + y * COLUMNS] != target) {
		neighbors[neighbor_count++] = (x - 1) + (y + 0) * COLUMNS;
	}
	if (x > 0 && y > 0 && grid[(x - 1) + (y - 1) * COLUMNS] != target) {
		neighbors[neighbor_count++] = (x - 1) + (y - 1) * COLUMNS;
	}

	for (uint32_t i = 0; i < neighbor_count; i++) {
		uint32_t neighbor_index = neighbors[i];

		size_t offset = arena_size(arena);
		flood_fill(arena, neighbor_index, target, grid);
		arena_set(arena, offset);
	}
	return true;
}
