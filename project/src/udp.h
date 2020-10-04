#ifndef UDP_H
#define UDP_H

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include "common.h"

int udpCreateSocket(const char *addressIP, const char *port);
int udpCreateClient(const char *addressIP, const char *port);
int udpCreateServer(const char *addressIP, const char *port);
int udpReceiveMessage(int fd, char *buffer, int mssgSize);
int udpSendMessage(int fd, const char *message, int mssgSize);
int udpShutdownSocket(int fd);

#endif /* UDP_H */
