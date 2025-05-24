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

	while (!WindowShouldClose()) {
		ClearBackground(RAYWHITE);

		int32_t x = roundf(player.position.x / (float)GRID_SIZE) * GRID_SIZE;
		int32_t y = roundf(player.position.y / (float)GRID_SIZE) * GRID_SIZE;

		if (IsKeyDown(KEY_W)) {
			player.direction.y = -1;
			player.direction.x = 0;
			player.position.x = x;
		}
		if (IsKeyDown(KEY_A)) {
			player.direction.x = -1;
			player.direction.y = 0;
			player.position.y = y;
		}
		if (IsKeyDown(KEY_S)) {
			player.direction.y = 1;
			player.direction.x = 0;
			player.position.x = x;
		}
		if (IsKeyDown(KEY_D)) {
			player.direction.x = 1;
			player.direction.y = 0;
			player.position.y = y;
		}
		float current_frame = GetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		player.position = Vector2Add(player.position, Vector2Multiply(Vector2Multiply(player.direction, player.speed), (Vector2){ delta_time, delta_time }));

		BeginDrawing();

		for (uint32_t y = 0; y < WINDOW_HEIGHT; y += GRID_SIZE) {
			for (uint32_t x = 0; x < WINDOW_WIDTH; x += GRID_SIZE) {
				DrawRectangleLines(x, y , GRID_SIZE, GRID_SIZE, GRAY);
			}
		}

		DrawRectangleV((Vector2){ .x = x, .y = y }, player.size, RED);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
