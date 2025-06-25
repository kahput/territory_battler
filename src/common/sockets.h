#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct _arena_handle Arena;
typedef struct _socket Socket;

Socket* socket_create(Arena* arena, int32_t domain, int32_t type);

bool socket_bind(Socket* sock, const char* address, int16_t port);
bool socket_listen(Socket* sock, int32_t backlog);
Socket* socket_accept(Arena* arena, Socket* sock);

bool socket_connect(Socket* sock, const char* address, int16_t port);
