#pragma once

#include "common/types.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct client Client;
typedef struct arena Arena;

Client* client_create(Arena* arena);
bool client_connect(Client* client, const char* address, uint16_t port);

void client_update(Arena* arena, Client* client);
bool client_poll(Client* client, NetEvent* event);

int32_t client_send(Client* client, void* data, uint32_t size);
