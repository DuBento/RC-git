#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#define MSSG_SIZE 128
#define PORT "58001"
#define TRUE 1

int main() {
	int fd, errcode, newfd;
	ssize_t n;
	socklen_t addrlen;
	struct addrinfo hints, *res;
	struct sockaddr_in addr;
	char buffer[MSSG_SIZE];

	/* Create socket. */
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	
	}
	/* Define info. */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	errcode = getaddrinfo(NULL, PORT, &hints, &res);
	if ((errcode) != 0) {
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	}

	n = bind(fd, res->ai_addr, res->ai_addrlen);
	if (n == -1) {
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	}	

	if (listen(fd, 5) == -1) {
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	}	


	while(TRUE) {
		addrlen = sizeof(addr);
		if ((newfd = accept(fd, (struct sockaddr*)&addr, &addrlen)) == -1){
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	
		}

		n = read(newfd, buffer, MSSG_SIZE);
		if (n == -1){
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	
		}

		write(1, "received: ", 10);
		write(1, buffer, n);

		n = write(newfd, buffer, n);
		if (n == -1){
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
		}


		close(newfd);
	}
	freeaddrinfo(res);
	close(fd);
}

