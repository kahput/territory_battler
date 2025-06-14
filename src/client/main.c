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
	Vector2 direction, speed;
} Player;

void print_player(Player p) {
	printf("Player {\n  position { x = %.2f, y = %.2f }\n  direction { x = %.2f, y = %.2f}\n}\n",
		p.position.x, p.position.y,
		p.direction.x, p.direction.y);
}

int main() {
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Land.io");

	Player player = {
		.position = { 3 * GRID_SIZE, 3 * GRID_SIZE },
		.size = { GRID_SIZE, GRID_SIZE },
		.speed = { 128, 128 }
	};

	float delta_time = 0.0f;
	float last_frame = 0.0f;

	Color colors[5] = { RAYWHITE, RED, BLUE, GREEN, ORANGE };
	uint32_t grid_target[GRID_SIZE * GRID_SIZE] = { 0 };

	while (!WindowShouldClose()) {
		ClearBackground(RAYWHITE);

		player.direction.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
		player.direction.y = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);

		float current_frame = GetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		player.direction = Vector2Normalize(player.direction);
		player.position = Vector2Add(player.position, Vector2Multiply(Vector2Multiply(player.direction, player.speed), (Vector2){ delta_time, delta_time }));

		BeginDrawing();

		for (uint32_t y = 0; y < WINDOW_HEIGHT / GRID_SIZE; y++) {
			for (uint32_t x = 0; x < WINDOW_WIDTH / GRID_SIZE; x++) {
				uint32_t index = x + y * (WINDOW_WIDTH / GRID_SIZE);
				DrawRectangle(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE, colors[grid_target[index]]);
				DrawRectangleLines(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE, GRAY);
			}
		}

		int32_t x = roundf(player.position.x / (float)GRID_SIZE) * GRID_SIZE;
		int32_t y = roundf(player.position.y / (float)GRID_SIZE) * GRID_SIZE;
		uint32_t index = (x / GRID_SIZE) + (y / GRID_SIZE) * (WINDOW_WIDTH / GRID_SIZE);

		grid_target[index] = 1;

		DrawRectangleV((Vector2){ x, y }, player.size, RED);
		DrawRectangleLinesEx((Rectangle){
								 .x = x,
								 .y = y,
								 .width = player.size.x,
								 .height = player.size.y },
			2, BLACK);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
