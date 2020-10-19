#include "../src/tcp.h"
#define MSSG_SIZE 128
#define PORT "58053"
#define IP "193.136.128.109"

int main() {
	int fd, errcode;
	ssize_t n;
	struct addrinfo hints, *res;
	char buffer[MSSG_SIZE];

	fd = tcpCreateClient(IP, PORT);

	errcode = tcpConnect(fd);
	
	errcode = tcpSendMessage(fd, "Hello!\n", 7);
	
	errcode = tcpReceiveMessage(fd, buffer, 7);
	
	write(1, "echo: ", 6);
	write(1, buffer, MSSG_SIZE);

	tcpDestroySocket(fd);
	exit(EXIT_SUCCESS);
}
