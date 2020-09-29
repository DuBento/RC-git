#include "../udp.h"

int main() {
    int fd;
    fd = udpCreateSocket("127.0.0.1", "5000");
    udpSendMessage(fd, "test\n", 5);
    udpShutdownSocket(fd);

    return 0;
}