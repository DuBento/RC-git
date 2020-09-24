#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define PORT "58001"

#define TRUE 1


int fd, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
char buffer[128];

int main() {
    /*struct addrinfo hints;
    int fd;*/

    fd = socket(AF_INET, SOCK_DGRAM, 0);        //UDP SOCKET
    if (fd == -1)
            exit(EXIT_FAILURE);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;              // IPv4
    hints.ai_family = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;


    errcode = getaddrinfo(NULL, PORT, &hints, &res);
    if ((errcode) != 0)
        exit(EXIT_FAILURE);

    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) /*im in*/
        exit(EXIT_FAILURE);

    while (TRUE) {
        int n;
        addrlen = sizeof(addr);
        n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
        if (n == -1)
            exit(EXIT_FAILURE);

        write(1, "received: ", 10);
        write(1, buffer, n);

        n = sendto(fd, buffer, n, 0, (struct sockaddr*)&addr, addrlen);
        if (n == -1)
            exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    close(fd);
}
