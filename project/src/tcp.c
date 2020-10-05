#include "tcp.h"

struct addrinfo hints, *res;


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
int tcpCreateSocket(const char *addressIP, const char *port) {
	int fd, errcode;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		fatal("TCP: Failed to create socket.");
	}
	
	/* Define info. */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	errcode = getaddrinfo(addressIP, port, &hints, &res);
	if ((errcode) != 0) {
		fatal("TCP: Failed to get address info");
	}
	return fd;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
int tcpCreateClient(const char *addressIP, const char *port) {
	return tcpCreateSocket(addressIP, port);
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */
int tcpCreateServer(const char *addressIP, const char *port, int numConnections) {
	int fd, errcode;
	fd = tcpCreateSocket(addressIP, port);
	
	errcode = bind(fd, res->ai_addr, res->ai_addrlen);
	if (errcode == -1) {
		fatal("TCP: Failed to bind server socket.");
	}	

	if (listen(fd, numConnections) == -1) {
		fatal("TCP: Failed to set listen");
	}	
	return fd;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
int tcpConnect(int fd) {
	int errcode;
	errcode = connect(fd, res->ai_addr, res->ai_addrlen);
	if (errcode == -1) {
		fatal("TCP: Failed to connect.");
	}
	return errcode;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
int tcpAcceptConnection(int fd) {
	int newfd, addrlen;
	struct sockaddr_in addr;

	addrlen = sizeof(addr); /* done every time? */
	if ((newfd = accept(fd, (struct sockaddr*)&addr, &addrlen)) == -1){
		fatal("TCP: Failed to accept.");
	}
	return newfd;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */
int tcpReceiveMessage(int fd, char *buffer, int mssgSize) {
	int n;
	n = read(fd, buffer, mssgSize);
	if (n == -1){
		fatal("TCP: Failed to read message.");	
	}
	return n;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */
int tcpSendMessage(int fd, const char *message, int mssgSize) {
	int n;
	
	n = write(fd, message, mssgSize);
	if (n == -1){
		fatal("TCP: Failed to send message.");	
	}
	return n;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
int tcpCloseConnection(int fd) {
	int errcode;
	errcode = close(fd);
	
	if (errcode == -1) {
		fatal("TCP: Failed to shutdown connection.");
	}
	return errcode;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
int tcpShutdownSocket(int fd) {
	int errcode;
	freeaddrinfo(res);
	errcode = close(fd);
	if (errcode == -1) {
		fatal("TCP: Failed to close socket.");
	}
	return errcode;
}
