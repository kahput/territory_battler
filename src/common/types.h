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
	MESSAGE_TYPE_WORLD_DATA,

	MESSAGE_TYPE_COUNT
} MessageType;

typedef struct message_header {
	MessageType type;
	int32_t rows, columns;
} MessageHeader;
