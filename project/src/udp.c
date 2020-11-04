#include "udp.h"


// creates and initializes an UDP socket
UDPConnection_t* udpCreateSocket(const char *addrIP, const char *port, char mode) {
	UDPConnection_t *udpConnection = (UDPConnection_t*)malloc(sizeof(UDPConnection_t));
	memset(&udpConnection->hints, '\0', sizeof(struct addrinfo));
	udpConnection->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpConnection->fd == -1)
		_FATAL("[UDP] Unable to create the socket!\n\t - Error code : %d", errno);

	udpConnection->hints.ai_family   = AF_INET;
	udpConnection->hints.ai_socktype = SOCK_DGRAM;    
	if (mode == SERVER)	udpConnection->hints.ai_flags = AI_PASSIVE;

	int errCode = getaddrinfo(addrIP, port, &udpConnection->hints, &udpConnection->res);
	if (errCode)
		_FATAL("[UDP] Unable to translate the the host name to an address with the "
		"getaddinfo() function!\n\t - Error code: %d", errCode);

	return udpConnection;
}


// creates an UDP server
UDPConnection_t* udpCreateServer(const char *addrIP, const char *port) {
	UDPConnection_t* udpConnection = udpCreateSocket(addrIP, port, SERVER);
	if (bind(udpConnection->fd, udpConnection->res->ai_addr, udpConnection->res->ai_addrlen)) {
		_FATAL("[UDP] Unable to bind the server.\n\t - Error: %s", strerror(errno));
	}
	return udpConnection;
}


// creates an UDP client
UDPConnection_t* udpCreateClient(const char *addrIP, const char *port) {
	return udpCreateSocket(addrIP, port, CLIENT);
}


// receives an UDP message
int udpReceiveMessage(UDPConnection_t *udpConnection, char *buffer, int len) {
	struct sockaddr *addr = { 0 };
	int addrlen = sizeof(addr);
	int n = recvfrom(udpConnection->fd, buffer, len-1, 0, addr, &addrlen);
	if (n == -1)
		_FATAL("[UDP] Unable to read the message!\n\t - Error code: %d", errno);

	buffer[n] = '\0'; // null terminate for string manipulation
	
	_LOG("[UDP] Message received (%d bytes) - '%s'", n, buffer);
	return n;
}


// sends an UDP message
int udpSendMessage(UDPConnection_t *udpConnection, const char *buffer, int len) {
	int n = sendto(udpConnection->fd, buffer, len, 0, udpConnection->res->ai_addr, udpConnection->res->ai_addrlen);
	if (n == -1)
		_FATAL("[UDP] Unable to send the message!\n\t - Error code: %d", errno);
	if (n != len)
		WARN("[UDP] Something went wrong with comunication.");
	_LOG("[UDP] Message sent (%d bytes) - '%s'", n, buffer);
	return n;
}

// populate struct and sends an UDP message
int udpSendMessage_specify(UDPConnection_t *udpConnection, const char *buffer, int len, char* ip, char* port) {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(atoi(port));

	int n = sendto(udpConnection->fd, buffer, len, 0, (const struct sockaddr *) &addr, sizeof(addr));
	if (n == -1)
		_FATAL("[UDP] Unable to send the message!\n\t - Error code: %d", errno);
	if (n != len)
		WARN("[UDP] Something went wrong with comunication.");
	_LOG("[UDP] Message sent (%d bytes) - '%s'", n, buffer);
	return n;
}

// terminates the udp socket
void udpDestroySocket(UDPConnection_t *udpConnection) {
	freeaddrinfo(udpConnection->res);
	if (close(udpConnection->fd))
		_FATAL("[UDP] Error while closing the socket!\n\t - Error code: %d", errno);
	free(udpConnection);
}
