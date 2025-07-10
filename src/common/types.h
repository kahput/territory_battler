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

	uint8_t* data;
	int32_t size;
} NetEvent;

typedef enum {
	MESSAGE_TYPE_NONE,
	MESSAGE_TYPE_DIRECTION,
	MESSAGE_TYPE_GAME_STATE,

	MESSAGE_TYPE_COUNT
} MessageType;

typedef struct {
	uint8_t color_id, client_id;
	int32_t x, y;
} PlayerState;

typedef struct {
	MessageType type;
	uint32_t rows, columns;
	uint32_t player_count;
} MessageHeader;
