#pragma once

#include <stdint.h>
typedef enum {
	NET_EVENT_NONE,
	NET_EVENT_CONNECT,
	NET_EVENT_DISCONNECT,
	NET_EVENT_RECEIVE,

	NET_EVENT_COUNT,
} NetEventType;

typedef struct net_event {
	NetEventType type;
	int32_t client_id;

	uint8_t *data;
	int32_t size;
} NetEvent;

