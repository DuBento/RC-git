#include "udp.h"
#include <netdb.h>
#include <stdio.h>



struct addrinfo hints={0}, *res={0};


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */
int udpCreateSocket(const char *addressIP, const char *port) {
    int fd, errcode;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
            fatal("Failed to create UDP socket.");
    }
    // not necessary because hints global var
    // memset(&hints, 0, sizeof(hints)); 
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    
    errcode = getaddrinfo(addressIP, port, &hints, &res);
    if( errcode != 0 )
        fatal("Failed to get address info");
    
    return fd;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */
int udpCreateServer(const char *addressIP, const char *port) {
    int fd, errcode;

    hints.ai_flags = AI_PASSIVE;
    fd = udpCreateSocket(addressIP, port);

    errcode = bind(fd, res->ai_addr, res->ai_addrlen);
    if (errcode == -1)
        fatal("Failed to create UDP server.\n");

    return fd;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */
int udpCreateClient(const char *addressIP, const char *port) {
    return udpCreateSocket(addressIP, port);
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
int udpSendMessage(int fd, const char *message, int mssgSize) {
    int n;

    n = sendto(fd, message, mssgSize, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1)
	        fatal("Fail to send UDP message.\n");
    return n;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
int udpReceiveMessage(int fd, char *buffer, int mssgSize) {
    
    int n;
    socklen_t addrlen;
    
    /* temporary */
    struct sockaddr *addr={0};
    addrlen = sizeof(addr);
    
    n = recvfrom(fd, buffer, mssgSize, 0, addr, &addrlen);

    /* DEBUG */
    _LOG("[UDP] Receive Message - return code: %d", n);

    if (n==-1) {
	        fatal("Fail to receive UDP message.\n");
    }
    return n;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \return Return parameter description
 */
int udpShutdownSocket(int fd) {
	int ret;
	freeaddrinfo(res);
	ret = close(fd);
	if (ret == -1)
		fatal("Failed to close UDP socket\n");
	return ret;
}
