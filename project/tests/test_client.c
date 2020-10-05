#include "../src/udp.h"
#include <stdio.h>

int main() {
    int fd;
    char buffer[BUFSIZ];
    int size;
    // fd = udpCreateSocket("193.136.138.142", "58011");
    fd = udpCreateSocket("localhost", "58011");
    udpSendMessage(fd, "test\n", 5);
    puts("sent");
    udpReceiveMessage(fd, buffer, size);
    fwrite(buffer, 1, size, stdin);
    udpShutdownSocket(fd);

    return 0;
}