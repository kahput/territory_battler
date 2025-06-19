#include "arena.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct _arena_handle {
	size_t size, capacity;
	void* data;
};

Arena* arena_alloc(void) {
	Arena* arena = malloc(sizeof(Arena));
	arena->size = 0;
	arena->capacity = (1 << 20);
	arena->data = malloc(arena->capacity);
	return arena;
}

void arena_clear(Arena* arena) {
	arena->size = 0;
}
void arena_free(Arena* arena) {
	free(arena->data);
	free(arena);
}

void* arena_push(Arena* arena, size_t size) {
	if (arena->size + size >= arena->capacity) {
		return NULL;
	}

	void* result = arena->data + arena->size;
	arena->size += size;

	return result;
}

void arena_pop(Arena* arena, size_t size) {
	arena->size -= size;
}
void arena_set(Arena* arena, size_t position) {
	arena->size = position;
}

size_t arena_size(Arena* arena) {
	return arena->size; 
}
