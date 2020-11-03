#include "udp.h"

static struct addrinfo hints = { 0 }, *res = NULL;

/* ===================== */
/* UDP General Fuctions */
/* ===================== */

// creates and initializes an UDP socket
int udpCreateSocket(const char *addrIP, const char *port) {
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		_FATAL("[UDP] Unable to create the socket!\n\t - Error code : %d", errno);

	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;    

	int errCode = getaddrinfo(addrIP, port, &hints, &res);
	if (errCode)
		_FATAL("[UDP] Unable to translate the the host name to an address with the "
		"getaddinfo() function!\n\t - Error code: %d", errCode);

	return fd;
}


// creates an UDP server
int udpCreateServer(const char *addrIP, const char *port) {
	hints.ai_flags = AI_PASSIVE;    
	int fd = udpCreateSocket(addrIP, port);
	if (bind(fd, res->ai_addr, res->ai_addrlen)) {
		_FATAL("[UDP] Unable to bind the server.\n\t - Error code: %d", errno);
	}
	return fd;
}


// creates an UDP client
int udpCreateClient(const char *addrIP, const char *port) {
	return udpCreateSocket(addrIP, port);
}


// receives an UDP message
int udpReceiveMessage(int fd, char *buffer, int len) {
	struct sockaddr *addr = { 0 };
	int addrlen = sizeof(addr);
	int n = recvfrom(fd, buffer, len-1, 0, addr, &addrlen);
	if (n == -1)
		_FATAL("[UDP] Unable to read the message!\n\t - Error code: %d", errno);

	buffer[n] = '\0'; // null terminate for string manipulation
	
	_LOG("[UDP] Message received (%d bytes) - '%s'", n, buffer);
	return n;
}


// sends an UDP message
int udpSendMessage(int fd, const char *buffer, int len) {
	int n = sendto(fd, buffer, len, 0, res->ai_addr, res->ai_addrlen);
	if (n == -1)
	    _FATAL("[UDP] Unable to send the message!\n\t - Error code: %d", errno);
	if (n != len)
		WARN("[UDP] Something went wrong with comunication.");
	_LOG("[UDP] Message sent (%d bytes) - '%s'", n, buffer);
	return n;
}

// populate struct and sends an UDP message
int udpSendMessage_specify(int fd, const char *buffer, int len, char* ip, char* port) {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &(addr.sin_addr));
	addr.sin_port = htons(atoi(port));

	int n = sendto(fd, buffer, len, 0, (const struct sockaddr *) &addr, sizeof(addr));
	if (n == -1)
		_FATAL("[UDP] Unable to send the message!\n\t - Error code: %d", errno);
	if (n != len)
		WARN("[UDP] Something went wrong with comunication.");
	_LOG("[UDP] Message sent (%d bytes) - '%s'", n, buffer);
	return n;
}

// terminates the udp socket
void udpDestroySocket(int fd) {
	freeaddrinfo(res);
	if (close(fd))
		_FATAL("[UDP] Error while closing the socket!\n\t - Error code: %d", errno);
}