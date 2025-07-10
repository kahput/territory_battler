#include "client/client.h"
#include "common/arena.h"
#include "common/socket.h"
#include "common/types.h"

#include <netinet/in.h>
#include <raylib.h>
#include <raymath.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 736
#define GRID_SIZE 32

static Color colors[5] = { RAYWHITE, RED, BLUE, GREEN, ORANGE };

typedef struct _game_state {
	Arena *permanent_arena, *frame_arena;
} GameState;

void draw_grid(uint8_t* grid_target, uint32_t rows, uint32_t columns);

int main(int32_t argc, char* argv[]) {
	if (argc > 1 && strcmp(argv[1], "headless") == 0)
		SetConfigFlags(FLAG_WINDOW_HIDDEN);

	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Land.io");

	float delta_time = 0.0f;
	float last_frame = 0.0f;

	GameState state = {
		.permanent_arena = arena_alloc(),
		.frame_arena = arena_alloc()
	};

	Client* client = client_create(state.permanent_arena);
	client_connect(client, "127.0.0.1", 5050);

	uint8_t* world_grid = NULL;
	uint32_t rows = 0, columns = 0;

	while (!WindowShouldClose()) {
		ClearBackground(RAYWHITE);
		arena_clear(state.frame_arena);

		client_update(state.frame_arena, client);

		NetEvent event;
		while (client_poll(client, &event)) {
			if (event.type == NET_EVENT_RECEIVE) {
				MessageHeader* header = (MessageHeader*)event.data;
				if (header->type == MESSAGE_TYPE_WORLD_DATA) {
					rows = ntohl(header->rows), columns = ntohl(header->columns);
					uint32_t size = rows * columns;
					printf("Got world data message of size %i\n", size);
					world_grid = arena_push_array_zero(state.permanent_arena, uint8_t, size);
					memcpy(world_grid, header + 1, size);
				}
			}
		}

		float current_frame = GetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		uint8_t new_direction = 0;
		if (IsKeyDown(KEY_W))
			new_direction = 1;
		if (IsKeyDown(KEY_D))
			new_direction = 2;
		if (IsKeyDown(KEY_S))
			new_direction = 3;
		if (IsKeyDown(KEY_A))
			new_direction = 4;

		uint8_t data[sizeof(MessageHeader) + sizeof(new_direction)];
		MessageHeader* header = (MessageHeader*)data;
		header->type = MESSAGE_TYPE_DIRECTION;
		*(data + sizeof(MessageHeader)) = new_direction;

		client_send(client, data, sizeof data);

		BeginDrawing();
		draw_grid(world_grid, rows, columns);
		EndDrawing();
	}

	arena_free(state.frame_arena);
	arena_free(state.permanent_arena);

	CloseWindow();
	return 0;
}

void draw_grid(uint8_t* grid_target, uint32_t rows, uint32_t columns) {
	for (uint32_t y = 0; y < rows; y++) {
		for (uint32_t x = 0; x < columns; x++) {
			uint32_t index = x + (y * columns);
			DrawRectangle(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE, colors[grid_target[index]]);
			DrawRectangleLines(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE, GRAY);
		}
	}
}
