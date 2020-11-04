#include "tcp.h"

static struct addrinfo hints = { 0 }, *res = NULL;



// creates an initializes a TCP socket
TCPConnection_t* tcpCreateSocket(const char *addrIP, const char *port) {
	TCPConnection_t *tcpConnection = (TCPConnection_t*)malloc(sizeof(TCPConnection_t));
	memset(&tcpConnection->hints, '\0', sizeof(struct addrinfo));
	tcpConnection->fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (tcpConnection->fd == -1)
		_FATAL("[TCP] Unable to create the socket!\n\t - Error code : %d", errno);
	
	tcpConnection->hints.ai_family   = AF_INET;
	tcpConnection->hints.ai_socktype = SOCK_STREAM;
	tcpConnection->hints.ai_flags    = AI_PASSIVE;

	int errCode = getaddrinfo(addrIP, port, &hints, &res);
	if (errCode)
		_FATAL("[TCP] Unable to translate the the host name to an address with the getaddinfo() function!\n"
		"\t - Error code: %d", errCode);

	return tcpConnection;
}


// creates a TCP server
TCPConnection_t* tcpCreateServer(const char *addrIP, const char *port, int nConnections) {
	TCPConnection_t *tcpConnection = tcpCreateSocket(addrIP, port);	
	if (bind(tcpConnection->fd, res->ai_addr, res->ai_addrlen))
		_FATAL("[TCP] Unable to bind the server.\n\t - Error code: %d", errno);

	if (listen(tcpConnection->fd, nConnections))
		_FATAL("[TCP] Unable to set the listed fd for the server.\n\t - Error code: %d", errno);

	return tcpConnection;
}


// creates a TCP client
TCPConnection_t* tcpCreateClient(const char *addrIP, const char *port) {
	return tcpCreateSocket(addrIP, port);
}


// connects the client with the server
void tcpConnect(TCPConnection_t *tcpConnection) {
	if (connect(tcpConnection->fd, res->ai_addr, res->ai_addrlen))
		_FATAL("[TCP] Unable to set the connect to the server.\n\t - Error code: %d", errno);
}


// accepts the connections from the clients
int tcpAcceptConnection(TCPConnection_t *tcpConnection) {	
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);

	int newfd = accept(tcpConnection->fd, (struct sockaddr*)&addr, &addrlen);
	if (newfd == -1)
		_FATAL("[TCP] Unable to accept a new connection.\n\t - Error code: %d", errno);

	return newfd;
}


// receives a TCP message
int tcpReceiveMessage(TCPConnection_t *tcpConnection, char *buffer, int len) {
	int sizeRead = 0;
	do {
		/* Upon successful completion, read() and pread() shall return a non-negative integer indicating the number of bytes actually read. 
		Otherwise, the functions shall return -1 and set errno to indicate the error. */
		int n = read(tcpConnection->fd, buffer, len);
		if (n == -1)
			_FATAL("[TCP] Unable to read the message!\n\t - Error code: %d", errno);	
		
		sizeRead += n;
		
	} while (buffer[sizeRead-1] != '\n');
	
	// Insert null char to be able to handle buffer content as a string.
	buffer[sizeRead] = '\0';
	return sizeRead;
}


// sends a TCP message
int tcpSendMessage(TCPConnection_t *tcpConnection, const char *buffer, int len) {
	int sizeWritten;

	sizeWritten = 0;
	do {
		/* On success, the number of bytes written is returned (zero indicates nothing was written). 
		On error, -1 is returned, and errno is set appropriately.*/
		int n = write(tcpConnection->fd, buffer, len);
		if (n == -1)
			_FATAL("[TCP] Unable to send the message!\n\t - Error code: %d", errno);
		sizeWritten += n;
	} while (sizeWritten != len);
	return sizeWritten;
}


// closes the tcp connection
void tcpCloseConnection(TCPConnection_t *tcpConnection) {	
	if (close(tcpConnection->fd))
		_FATAL("[TCP] Error while terminating the connection!\n\t - Error code: %d", errno);
}


// terminates the tcp socket
void tcpDestroySocket(TCPConnection_t *tcpConnection) {
	freeaddrinfo(res);
	if (close(tcpConnection->fd))
		_FATAL("[TCP] Error while closing the socket!\n\t - Error code: %d", errno);
}