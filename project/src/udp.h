#ifndef UDP_H
#define UDP_H

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "common.h"

typedef struct udp_node_t {
    struct sockaddr *ai_addr;
    socklen_t ai_addrlen;
} udpNode_t;


/* Structure that stores the information for the UDP connection. */
typedef struct udp_connection {
    int fd;
    struct addrinfo hints;
    struct addrinfo *res;
} UDPConnection_t;


/*! \brief Creates and initializes an UDP socket.
 *
 *  Creates a new socket for an UDP connection with the specified IP address
 *  and port number (after translating them into the appropriate formats).
 *
 *  \param  addrIP	the ip address (IPv4 or IPv6).
 *  \param  port	the service name (or the port number).
 *  \return the udp connection structure.
 */
UDPConnection_t* udpCreateSocket(const char *addrIP, const char *port);


/*! \brief Creates an UDP server.
 *
 *  Creates an UDP server with the specified IP address and port number.
 *
 *  \param  addrIP	the ip address (IPv4 or IPv6).
 *  \param  port	the service name (or the port number).
 *  \return the udp connection structure.
 */
UDPConnection_t* udpCreateServer(const char *addrIP, const char *port);


/*! \brief Creates an UDP client.
 *
 *  Creates an UDP client with the specified IP address and port number.
 *
 *  \param  addrIP	the ip address (IPv4 or IPv6).
 *  \param  port		the service name (or the port number).
 *  \return the udp connection structure.
 */
UDPConnection_t* udpCreateClient(const char *addrIP, const char *port);


/*! \brief Receives an UDP message.
 *
 *  Stores an UDP message in the specified buffer.
 *
 *  \param  udpConnection	the udp connection structure.
 *  \param  buffer	        a buffer where the message will be stored.
 *  \param  len		        the length of the specified buffer.
 *  \return the number of bytes read.
 */
int udpReceiveMessage(UDPConnection_t *udpConnection, char *buffer, int len);


/*! \brief Sends an UDP message.
 *
 *  Sends the specified UDP message.
 *
 *  \param  udpConnection	the udp connection structure.
 *  \param  buffer	        a buffer containing the message.
 *  \param  len		        the length of the message.
 *  \return the number of bytes sent.
 */
int udpSendMessage(UDPConnection_t *udpConnection, const char *buffer, int len);


/*! \brief 
 *
 *  
 *
 *  \param  udpConnection	the udp connection structure.
 *  \param  buffer	        a buffer containing the message.
 *  \param  len		        the length of the message.
 *  \param  ip
 *  \param  port
 *  \return the number of bytes sent.]
 */
int udpSendMessage_specify(UDPConnection_t *udpConnection, const char *buffer, int len, char* ip, char* port);


/*! \brief Terminates the UDP socket.
 *
 *  Frees the information associated with an UDP socket and closes it.
 *
 *  \param  udpConnection	the udp connection structure.
 */
void udpDestroySocket(UDPConnection_t *udpConnection);



#endif  /* UDP_H */