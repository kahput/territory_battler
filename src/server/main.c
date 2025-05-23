#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "8080"

int main() {
	int32_t server_fd, client_fd;
	struct addrinfo hints, *server_addr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int32_t status;
	if ((status = getaddrinfo(NULL, PORT, &hints, &server_addr)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return -1;
	}

	if ((server_fd = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol)) < 0) {
		printf("Server failed to create socket\n");
		close(server_fd);
		freeaddrinfo(server_addr);
		return -1;
	}

	if (bind(server_fd, server_addr->ai_addr, server_addr->ai_addrlen) < 0) {
		printf("Server failed to bind on port %s\n", PORT);
		close(server_fd);
		freeaddrinfo(server_addr);
		return -1;
	}

	if (listen(server_fd, 5) < 0) {
		printf("Server failed to listen on port %s\n", PORT);
		close(server_fd);
		freeaddrinfo(server_addr);
		return -1;
	}

	printf("Server listening on port %s...\n", PORT);

	if ((client_fd = accept(server_fd, NULL, NULL)) < 0) {
		printf("Server failed to accept connect on port %s\n", PORT);
		close(server_fd);
		freeaddrinfo(server_addr);
		return -1;
	}

	char message[] = "Hello from server!";
	send(client_fd, message, sizeof(message), 0);

	close(server_fd);
	close(client_fd);
	return 0;
}
