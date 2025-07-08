#pragma once
#include "common/types.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct server Server;
typedef struct arena Arena;

Server* server_create(Arena* arena, uint16_t port, int32_t max_clients);
void server_update(Arena* arena, Server* server);
bool server_poll(Server* server, NetEvent* event);

void server_send(Server* server, int32_t client_id, void* data, int32_t size);
void server_broadcast(Server* server,  void* data, int32_t size);
