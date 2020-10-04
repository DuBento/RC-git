#include "udp.h"



struct addrinfo hints, *res;

/** \brief Creates an UDP socket.
 * 
 * 	blah blah blah please detail me :)
 * 
 * 	\param 	userInfo
 *          a pointer to store the user's info.
 *  \return the new socket fd.
 */
int udpCreateSocket(const char *addressIP, const char *port) {
    int fd, errcode;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
            fatal("Failed to create UDP socket.");
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    
    errcode = getaddrinfo(addressIP, port, &hints, &res);
    if( errcode != 0 )
        fatal("Failed to get address info");
    
    return fd;
}

/** \brief <short description>.
 * 
 * 	<long description> blah blah blah please detail me :)
 * 
 * 	\param 	<param name>
 *          <param description>.
 *
 *  \return <what it returns>.
 */

int udpCreateServer(const char *addressIP, const char *port) {
    int fd, errcode;

    fd = udpCreateSocket(addressIP, port);
    errcode = bind(fd, res->ai_addr, res->ai_addrlen);
    if (errcode == -1)
        fatal("Failed to create UDP server.\n");

    return fd;
}

/** \brief <short description>.
 * 
 * 	<long description> blah blah blah please detail me :)
 * 
 * 	\param 	<param name>
 *          <param description>.
 *
 *  \return <what it returns>.
 */

int udpCreateClient(const char *addressIP, const char *port) {
    return udpCreateSocket(addressIP, port);
}

/** \brief <short description>.
 * 
 * 	<long description> blah blah blah please detail me :)
 * 
 * 	\param 	<param name>
 *          <param description>.
 *
 *  \return <what it returns>.
 */

int udpSendMessage(int fd, const char *message, int mssgSize) {
    int n;

    n = sendto(fd, message, mssgSize, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1)
	        fatal("Fail to send UDP message.\n");
    return n;
}

/** \brief <short description>.
 * 
 * 	<long description> blah blah blah please detail me :)
 * 
 * 	\param 	<param name>
 *          <param description>.
 *
 *  \return <what it returns>.
 */

int udpReceiveMessage(int fd, char *buffer, int mssgSize) {
    
    int n;
    socklen_t addrlen;
    
    /* temporary */
    struct sockaddr *addr;
    
    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer, mssgSize, 0, addr, &addrlen);
    if (n==-1) {
	        fatal("Fail to receive UDP message.\n");
    }
    return n;
}

/** \brief <short description>.
 * 
 * 	<long description> blah blah blah please detail me :)
 * 
 * 	\param 	<param name>
 *          <param description>.
 *
 *  \return <what it returns>.
 */

int udpShutdownSocket(int fd) {
	int ret;
	freeaddrinfo(res);
	ret = close(fd);
	if (ret == -1)
		fatal("Failed to close UDP socket\n");
	return ret;
}
