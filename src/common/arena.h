#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct _arena_handle Arena;

Arena* arena_alloc(void);
void arena_clear(Arena* arena);
void arena_free(Arena* arena);

void* arena_push(Arena* arena, size_t size);
#define arena_push_array(arena, type, count) (type*)arena_push((arena), sizeof(type) * (count))
#define arena_push_type(arena, type) (type*)arena_push((arena), sizeof(type));

void arena_pop(Arena* arena, size_t size);
void arena_set(Arena* arena, size_t position);

size_t arena_size(Arena* arena);
