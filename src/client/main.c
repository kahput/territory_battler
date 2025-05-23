#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#define PORT "8080"

int main() {
	int32_t client_fd = 0;
	struct addrinfo hints, *server_addr;
	char message[1024] = { 0 };

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;

	int32_t status;
	if ((status = getaddrinfo("localhost", PORT, &hints, &server_addr)) != 0) {
		printf("getaddrinfo error: %s\n", gai_strerror(status));
		freeaddrinfo(server_addr);
		return -1;
	}

	if ((client_fd = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol)) < 0) {
		printf("Failed to create socket\n");
		freeaddrinfo(server_addr);
		return -1;
	}

	if (connect(client_fd, server_addr->ai_addr, server_addr->ai_addrlen) < 0) {
		printf("Connect error: %s\n", strerror(errno)); // Show actual error
		printf("Failed to connect to server on port %s\n", PORT);
		close(client_fd);
		freeaddrinfo(server_addr);
		return -1;
	}

	freeaddrinfo(server_addr);

	read(client_fd, message, sizeof(message));
	printf("Message from server: %s\n", message);

	close(client_fd);
}
