#include "client/client.h"
#include "common/arena.h"
#include "common/types.h"

#include <raylib.h>
#include <raymath.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 736
#define GRID_SIZE 32
#define ROWS (WINDOW_HEIGHT / GRID_SIZE)
#define COLUMNS (WINDOW_WIDTH / GRID_SIZE)

typedef struct _player {
	Vector2 position, size;
	int32_t direction, speed, color_id;
} Player;

static Color colors[5] = { RAYWHITE, RED, BLUE, GREEN, ORANGE };

typedef struct _game_state {
	Arena *permanent_arena, *frame_arena;
} GameState;

uint32_t owned_neighbor_count(uint32_t target, uint32_t index);
void draw_grid(uint8_t grid_target[ROWS * COLUMNS]);
bool encolsure_fill(Arena *arena, uint32_t index, uint32_t target, uint8_t grid[ROWS * COLUMNS]);

Vector2 to_vec2(int32_t direction);
void print_player(Player p);

int main(int32_t argc, char *argv[]) {
	if (argc > 1 && strcmp(argv[1], "headless") == 0)
		SetConfigFlags(FLAG_WINDOW_HIDDEN);

	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Land.io");

	Player player = {
		.position = { 3 * GRID_SIZE, 3 * GRID_SIZE },
		.size = { GRID_SIZE, GRID_SIZE },
		.speed = 128,
		.color_id = 1
	};

	float delta_time = 0.0f;
	float last_frame = 0.0f;

	GameState state = {
		.permanent_arena = arena_alloc(),
		.frame_arena = arena_alloc()
	};

	Client *client = client_create(state.permanent_arena);
	client_connect(client, "127.0.0.1", 5050);

	uint8_t *grid_target = arena_push_array_zero(state.permanent_arena, uint8_t, ROWS *COLUMNS);
	float step_timer = 0;

	while (!WindowShouldClose()) {
		ClearBackground(RAYWHITE);
		arena_clear(state.frame_arena);

		client_update(state.frame_arena, client);

		NetEvent event;
		while (client_poll(client, &event)) {
			if (event.type == NET_EVENT_RECEIVE) {
				printf("Message: %s\n", event.data);

				char answer[1024];
				snprintf(answer, 1024, "Client ACK message [%s]", event.data);

				client_send(client, answer, sizeof answer);
			}
		}

		float current_frame = GetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		step_timer += player.speed * delta_time;

		if (step_timer >= 32 && player.direction != 0) {
			Vector2 new_position = Vector2Add(player.position, Vector2Scale(to_vec2(player.direction), GRID_SIZE));
			if (new_position.x >= 0 && (int)(new_position.x / GRID_SIZE) < COLUMNS)
				player.position.x = new_position.x;
			if (new_position.y >= 0 && (int)(new_position.y / GRID_SIZE) < ROWS)
				player.position.y = new_position.y;
			step_timer = 0;
		}

		int32_t new_direction = 0;
		if (IsKeyDown(KEY_W))
			new_direction = 1;
		if (IsKeyDown(KEY_D))
			new_direction = 2;
		if (IsKeyDown(KEY_S))
			new_direction = 3;
		if (IsKeyDown(KEY_A))
			new_direction = 4;
		if (IsKeyPressed(KEY_SPACE))
			player.direction = 0;

		if (new_direction != 0 && player.direction != new_direction) {
			step_timer = 0;
			player.direction = new_direction;
		}

		BeginDrawing();

		draw_grid(grid_target);

		uint32_t index = (int32_t)(player.position.x / GRID_SIZE) + (int32_t)(player.position.y / GRID_SIZE) * COLUMNS;
		grid_target[index] = player.color_id;
		encolsure_fill(state.frame_arena, index, player.color_id, grid_target);

		DrawRectangleV((Vector2){ player.position.x, player.position.y }, player.size, colors[player.color_id]);
		DrawRectangleLinesEx((Rectangle){
								 .x = player.position.x,
								 .y = player.position.y,
								 .width = player.size.x,
								 .height = player.size.y },
			2, BLACK);

		EndDrawing();
	}

	arena_free(state.frame_arena);
	arena_free(state.permanent_arena);

	CloseWindow();
	return 0;
}

Vector2 to_vec2(int32_t direction) {
	return (Vector2){
		.x = direction == 2 ? 1 : direction == 4 ? -1
												 : 0,
		.y = direction == 1 ? -1 : direction == 3 ? 1
												  : 0,
	};
}

void print_player(Player p) {
	printf("Player {\n  position { x = %.2f, y = %.2f }\n}\n",
		p.position.x, p.position.y);
}

void draw_grid(uint8_t grid_target[ROWS * COLUMNS]) {
	for (uint32_t y = 0; y < ROWS; y++) {
		for (uint32_t x = 0; x < COLUMNS; x++) {
			uint32_t index = x + (y * COLUMNS);
			DrawRectangle(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE, colors[grid_target[index]]);
			DrawRectangleLines(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE, GRAY);
		}
	}
}

void flood_fill(Arena *arena, uint32_t index, uint32_t target, uint8_t grid[ROWS * COLUMNS]) {
	int32_t *stack = arena_push_array(arena, int32_t, ROWS *COLUMNS);
	bool *visisted = arena_push_array(arena, bool, ROWS *COLUMNS);
	uint32_t *result = arena_push_array(arena, uint32_t, ROWS *COLUMNS);
	memset(visisted, 0, ROWS * COLUMNS);
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
