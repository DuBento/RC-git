// lixo

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#define PORT    "58053"
#include <stdio.h>



int main(){
    int fd, errcode, len;
    struct addrinfo hints, *res;
    ssize_t n;
    socklen_t addrlen;
    char buffer[128];
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1) exit(1);

    memset(&hints, 0, sizeof(hints));
    memset(buffer, 0, 128);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    
    errcode = getaddrinfo(NULL, PORT, &hints, &res);
    if(errcode != 0) exit(1);


    n = sendto(fd, "mensagem enviada pelo cliente\n", 30, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) exit(1);

    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
    if (n==-1) exit(1);

    write(1, "echo: ", 6); write(1, buffer, n);

    freeaddrinfo(res);
    close(fd);
}

