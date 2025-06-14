#include <math.h>
#include <raylib.h>
#include <raymath.h>

#include <stdint.h>
#include <stdio.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 736
#define GRID_SIZE 32

typedef struct _player {
	Vector2 position, size;
	int direction, speed;
} Player;

void draw_grid(uint32_t grid_target[GRID_SIZE * GRID_SIZE]);

Vector2 to_vec2(int32_t direction);
void print_player(Player p);

int main() {
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Land.io");

	Player player = {
		.position = { 3 * GRID_SIZE, 3 * GRID_SIZE },
		.size = { GRID_SIZE, GRID_SIZE },
		.speed = 128
	};

	float delta_time = 0.0f;
	float last_frame = 0.0f;

	uint32_t grid_target[GRID_SIZE * GRID_SIZE] = { 0 };

	float step_timer = 0;

	while (!WindowShouldClose()) {
		ClearBackground(RAYWHITE);

		float current_frame = GetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		step_timer += player.speed * delta_time;

		if (step_timer >= 32 && player.direction != 0) {
			player.position = Vector2Add(player.position, Vector2Scale(to_vec2(player.direction), GRID_SIZE));
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

		if (new_direction != 0 && player.direction != new_direction) {
			step_timer = 0;
			player.direction = new_direction;
		}

		BeginDrawing();

		draw_grid(grid_target);

		uint32_t index = (player.position.x / GRID_SIZE) + (player.position.y / GRID_SIZE) * ((float)WINDOW_WIDTH / GRID_SIZE);
		grid_target[index] = 1;

		DrawRectangleV((Vector2){ player.position.x, player.position.y }, player.size, RED);
		DrawRectangleLinesEx((Rectangle){
								 .x = player.position.x,
								 .y = player.position.y,
								 .width = player.size.x,
								 .height = player.size.y },
			2, BLACK);

		EndDrawing();
	}

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

void draw_grid(uint32_t grid_target[GRID_SIZE * GRID_SIZE]) {
	uint32_t rows = WINDOW_HEIGHT / GRID_SIZE, columns = WINDOW_WIDTH / GRID_SIZE;
	static Color colors[5] = { RAYWHITE, RED, BLUE, GREEN, ORANGE };

	for (uint32_t y = 0; y < rows; y++) {
		for (uint32_t x = 0; x < columns; x++) {
			uint32_t index = x + y * columns;
			DrawRectangle(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE, colors[grid_target[index]]);
			DrawRectangleLines(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE, GRAY);
		}
	}
}
