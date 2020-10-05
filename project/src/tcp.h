#ifndef TCP_H
#define TCP_H

#include "common.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int tcpCreateSocket(const char *addressIP, const char *port);
int tcpCreateClient(const char *addressIP, const char *port);
int tcpCreateServer(const char *addressIP, const char *port, int numConnections);
int tcpConnect(int fd);
int tcpAcceptConnection(int fd);
int tcpReceiveMessage(int fd, char *buffer, int mssgSize);
int tcpSendMessage(int fd, const char *message, int mssgSize);
int tcpCloseConnection(int fd);
int tcpShutdownSocket(int fd);

#endif /* TCP_H */
