#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct arena Arena;
typedef struct _socket Socket;

Socket* socket_create(Arena* arena, int32_t domain, int32_t type);

bool socket_bind(Socket* sock, const char* address, int16_t port);
bool socket_listen(Socket* sock, int32_t backlog);
Socket* socket_accept(Arena* arena, Socket* sock);

int32_t socket_fd(Socket* sock);

bool socket_connect(Socket* sock, const char* address, uint16_t port);

// void socket_track(Socket* socket);
// NetEvent* socket_poll(Arena* arena);

int32_t socket_send(Socket* sock, const int8_t* data, uint32_t size);
int32_t socket_recv(Socket* sock, void* data, uint32_t size);
