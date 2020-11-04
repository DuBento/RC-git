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
	if (bind(fd, res->ai_addr, res->ai_addrlen))
		_FATAL("[TCP] Unable to bind the server.\n\t - Error code: %d", errno);

	if (listen(fd, nConnections))
		_FATAL("[TCP] Unable to set the listed fd for the server.\n\t - Error code: %d", errno);

	return fd;
}


// creates a TCP client
int tcpCreateClient(const char *addrIP, const char *port) {
	return tcpCreateSocket(addrIP, port);
}


// connects the client with the server
void tcpConnect(int fd) {
	if (connect(fd, res->ai_addr, res->ai_addrlen))
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
	int sizeRead = 0;
	do {
		/* Upon successful completion, read() and pread() shall return a non-negative integer indicating the number of bytes actually read. 
		Otherwise, the functions shall return -1 and set errno to indicate the error. */
		int n = read(fd, buffer, len);
		if (n == -1)
			_FATAL("[TCP] Unable to read the message!\n\t - Error code: %d", errno);	
		
		sizeRead += n;
		
	} while (buffer[sizeRead-1] != '\n');
	
	// Insert null char to be able to handle buffer content as a string.
	buffer[sizeRead] = '\0';
	return sizeRead;
}


// sends a TCP message
int tcpSendMessage(int fd, const char *buffer, int len) {
	int sizeWritten;

	sizeWritten = 0;
	do {
		/* On success, the number of bytes written is returned (zero indicates nothing was written). 
		On error, -1 is returned, and errno is set appropriately.*/
		int n = write(fd, buffer, len);
		if (n == -1)
			_FATAL("[TCP] Unable to send the message!\n\t - Error code: %d", errno);
		sizeWritten += n;
	} while (sizeWritten != len);
	return sizeWritten;
}


// closes the tcp connection
void tcpCloseConnection(int fd) {	
	if (close(fd))
		_FATAL("[TCP] Error while terminating the connection!\n\t - Error code: %d", errno);
}


// terminates the tcp socket
void tcpDestroySocket(int fd) {
	freeaddrinfo(res);
	if (close(fd))
		_FATAL("[TCP] Error while closing the socket!\n\t - Error code: %d", errno);
}