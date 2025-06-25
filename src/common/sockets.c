#include "sockets.h"
#include "arena.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct _socket {
	int32_t domain, type, file_descriptor;
	struct sockaddr_storage address;
};

Socket* socket_create(Arena* arena, int32_t domain, int32_t type) {
	Socket* sock = arena_push_type(arena, struct _socket);
	sock->file_descriptor = 0;
	sock->domain = domain;
	sock->type = type;
	return sock;
}

bool socket_bind(Socket* sock, const char* address, int16_t port) {
	struct addrinfo hints, *server_info, *p;
	int32_t yes = 1, result;
	socklen_t sin_size;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = sock->domain;
	hints.ai_socktype = sock->type;
	if (address == NULL)
		hints.ai_flags = AI_PASSIVE;

	char str_port[5];
	sprintf(str_port, "%d", port);
	str_port[4] = '\0';

	if ((result = getaddrinfo(address, str_port, &hints, &server_info)) != 0) {
		fprintf(stderr, "ERROR: %s", gai_strerror(result));
		return result;
	}

	for (p = server_info; p != NULL; p = p->ai_next) {
		if ((sock->file_descriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("socket_bind -> socket()");
			continue;
		}

		if (setsockopt(sock->file_descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
			perror("socket_bind -> setsockopt()");
			return false;
		}

		if (bind(sock->file_descriptor, p->ai_addr, p->ai_addrlen) == -1) {
			close(sock->file_descriptor);
			perror("socket_bind -> bind()");
			continue;
		}

		break;
	}

	freeaddrinfo(server_info);
	if (p == NULL) {
		close(sock->file_descriptor);
		fprintf(stderr, "Server: failed to bind\n");
		return false;
	}

	return true;
}

bool socket_listen(Socket* sock, int32_t backlog) {
	if (listen(sock->file_descriptor, backlog) == -1) {
		close(sock->file_descriptor);
		perror("listen()");
		return false;
	}
	return true;
}

void* to_in_addr(struct sockaddr* sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

Socket* socket_accept(Arena* arena, Socket* server_socket) {
	Socket* client_socket = arena_push_type(arena, Socket);
	struct sockaddr_storage* client_info = &client_socket->address;
	char address_buffer[INET6_ADDRSTRLEN];
	socklen_t sin_size = sizeof(client_socket->address);

	client_socket->file_descriptor = accept(server_socket->file_descriptor, (struct sockaddr*)client_info, &sin_size);
	if (client_socket->file_descriptor == -1) {
		perror("accept()");
		return NULL;
	}
	inet_ntop(client_info->ss_family, to_in_addr((struct sockaddr*)client_info), address_buffer, sizeof address_buffer);
	printf("Server: connection from %s\n", address_buffer);

	return client_socket;
}

bool socket_connect(Socket* sock, const char* address, int16_t port) {
	struct addrinfo hints, *server_info, *p;
	int32_t yes = 1, result;
	socklen_t sin_size;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = sock->domain;
	hints.ai_socktype = sock->type;

	char str_port[5];
	sprintf(str_port, "%d", port);
	str_port[4] = '\0';

	if ((result = getaddrinfo(address, str_port, &hints, &server_info)) != 0) {
		fprintf(stderr, "ERROR: %s", gai_strerror(result));
		return result;
	}

	for (p = server_info; p != NULL; p = p->ai_next) {
		if ((sock->file_descriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("socket_connect -> socket()");
			continue;
		}

		if (setsockopt(sock->file_descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
			perror("socket_connect -> setsockopt()");
			return false;
		}

		char address_buffer[INET6_ADDRSTRLEN];

		inet_ntop(server_info->ai_family, server_info->ai_addr, address_buffer, sizeof address_buffer);
		printf("Client: attempting connection to %s\n", address_buffer);

		if (connect(sock->file_descriptor, p->ai_addr, p->ai_addrlen) == -1) {
			close(sock->file_descriptor);
			perror("socket_connect -> bind()");
			continue;
		}

		break;
	}

	if (p == NULL) {
		close(sock->file_descriptor);
		fprintf(stderr, "client: failed to connect\n");
		return false;
	}
	char address_buffer[INET6_ADDRSTRLEN];
	inet_ntop(server_info->ai_family, server_info->ai_addr, address_buffer, sizeof address_buffer);
		printf("Client: connected to %s\n", address_buffer);

	freeaddrinfo(server_info);

	return true;
}
