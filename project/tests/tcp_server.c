#include "../src/tcp.h"

#define MSSG_SIZE 128
#define PORT "58053"
#define NUM_CONNECTIONS 5
#define IP NULL

int main() {
	int fd, errcode, newfd, sz;
	char buffer[MSSG_SIZE];

	/* Create socket. */
	fd = tcpCreateServer(IP, PORT, NUM_CONNECTIONS);

	while(TRUE) {
		printf("Server waiting...\n");
		newfd = tcpAcceptConnection(fd);

		sz = tcpReceiveMessage(newfd, buffer, MSSG_SIZE);

		write(1, "received: ", 10);
		write(1, buffer, sz);

		sz = tcpSendMessage(newfd, buffer, MSSG_SIZE);

		tcpCloseConnection(newfd);
	}
	tcpDestroySocket(fd);
	exit(EXIT_SUCCESS);
}

