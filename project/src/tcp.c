#include "tcp.h"

static struct addrinfo hints = { 0 }, *res = NULL;


// creates an initializes a TCP socket
int tcpCreateSocket(const char *addrIP, const char *port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		_FATAL("[TCP] Unable to create the socket!\n\t - Error code : %d", errno);
	
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;

	int errCode = getaddrinfo(addrIP, port, &hints, &res);
	if (errCode)
		_FATAL("[TCP] Unable to translate the the host name to an address with the getaddinfo() function!\n"
            "\t - Error code: %d", errCode);

	return fd;
}


// creates a TCP server
int tcpCreateServer(const char *addrIP, const char *port, int nConnections) {
	int fd = tcpCreateSocket(addrIP, port);	
	if (bind(fd, res->ai_addr, res->ai_addrlen) == -1)
		_FATAL("[TCP] Unable to bind the server.\n\t - Error code: %d", errno);

	if (listen(fd, nConnections) == -1)
		_FATAL("[TCP] Unable to set the listed fd for the server.\n\t - Error code: %d", errno);

	return fd;
}


// creates a TCP client
int tcpCreateClient(const char *addrIP, const char *port) {
	return tcpCreateSocket(addrIP, port);
}


// connects the client with the server
void tcpConnect(int fd) {
	if (connect(fd, res->ai_addr, res->ai_addrlen) == -1)
		_FATAL("[TCP] Unable to set the connect to the server.\n\t - Error code: %d", errno);
}


// accepts the connections from the clients
int tcpAcceptConnection(int fd) {	
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);

	int newfd = accept(fd, (struct sockaddr*)&addr, &addrlen);
	if (newfd == -1)
		_FATAL("[TCP] Unable to accept a new connection.\n\t - Error code: %d", errno);

	return newfd;
}


// receives a TCP message
int tcpReceiveMessage(int fd, char *buffer, int len) {
	int n = read(fd, buffer, len);
	if (n == -1)
		_FATAL("[TCP] Unable to read the message!\n\t - Error code: %d", errno);	
	
	return n;
}


// sends a TCP message
int tcpSendMessage(int fd, const char *buffer, int len) {
	int n = write(fd, buffer, len);
	if (n == -1)
		_FATAL("[TCP] Unable to send the message!\n\t - Error code: %d", errno);
	
	return n;
}


// closes the tcp connection
void tcpCloseConnection(int fd) {	
	if (close(fd) == -1)
		_FATAL("[TCP] Error while terminating the connection!\n\t - Error code: %d", errno);
}


// terminates the tcp socket
void tcpDestroySocket(int fd) {
	freeaddrinfo(res);
	if (close(fd) == -1)
		_FATAL("[TCP] Error while closing the socket!\n\t - Error code: %d", errno);
}
