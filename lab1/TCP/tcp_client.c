#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>


#include <stdio.h>

#define PORT "58801"
#define MSSG_SIZE 128

int fd, errcode;
ssize_t n;
struct addrinfo hints, *res;
char buffer[MSSG_SIZE];


int main() {
	
/*	int fd, errcode;
	ssize_t n;
	struct addrinfo hints, *res;
	char buffer[MSSG_SIZE];*/

	fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (fd == -1) {
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	errcode = getaddrinfo("sigma03.tecnico.ulisboa.pt", PORT, &hints, &res);
	if (errcode == -1) {
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	}

	n = connect(fd, res->ai_addr, res->ai_addrlen);
	
	if (n == -1) {
		fprintf(stderr, "Failed to connect client.\n");
		exit(EXIT_FAILURE);
	}
	
	n = write(fd, "Hello!\n", 7);
	
	if (n == -1) {
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	}

	n = read(fd, buffer, MSSG_SIZE);
	
	if (n == -1) {
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	}

	write(1, "echo: ", 6);
	write(1, buffer, n);

	freeaddrinfo(res);
	close(fd);

	exit(EXIT_SUCCESS);
}
