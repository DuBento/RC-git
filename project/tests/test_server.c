#include "../udp.h"
#include <stdio.h>

int main() {
    int fd;
    char buffer[512];

    fd = udpCreateServer("localhost", "5000");
    udpReceiveMessage(fd, buffer, 512);
    printf("%s", buffer);
    
    return 0;
}