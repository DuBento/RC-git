#ifndef TCP_H
#define TCP_H

#include "common.h"


/* Structure that stores the information for the UDP connection. */
typedef struct tcp_connection {
    int fd;
    struct addrinfo hints;
    struct addrinfo *res;
} TCPConnection_t;



/*! \brief Creates and initializes a TCP socket.
 *
 *  Creates a new socket for a TCP connection with the specified IP address
 *  and port number (after translating them into the appropriate formats).
 *
 *  \param  addrIP  	the ip address (IPv4 or IPv6).
 *  \param  port	    the service name (or the port number).
 *  \return the tcp connection structure.
 */
TCPConnection_t* tcpCreateSocket(const char *addrIP, const char *port);


/*! \brief Creates an TCP server.
 *
 *  Creates an TCP server with the specified IP address and port number.
 *
 *  \param  addrIP	        the ip address (IPv4 or IPv6).
 *  \param  port		    the service name (or the port number).
 *  \param  nConnections	the maximum number of connections in queue.
 *  \return the tcp connection structure.
 */
TCPConnection_t* tcpCreateServer(const char *addrIP, const char *port, int nConnections);


/*! \brief Creates an TCP client.
 *
 *  Creates an TCP client with the specified IP address and port number.
 *
 *  \param  addrIP	the ip address (IPv4 or IPv6).
 *  \param  port		the service name (or the port number).
 *  \return the tcp connection structure.
 */
TCPConnection_t* tcpCreateClient(const char *addrIP, const char *port);


/*! \brief Connects the client with the server.
 *
 *  Tries to connect to the server.
 *
 *  \param  tcpConnection	the tcp connection structure.
 */
void tcpConnect(TCPConnection_t *udpConnection);


/*! \brief Accepts the connections from the clients.
 *
 *  Accepts a new connection from a clients and returns a new file descriptor
 *  for communicating with it.
 *
 *  \param  tcpConnection	the tcp connection structure.
 *  \return the file descriptor for communicating with the client.
 */
int tcpAcceptConnection(TCPConnection_t *udpConnection);


/*! \brief Receives a TCP message.
 *
 *  Stores a TCP message in the specified buffer.
 *
 *  \param  tcpConnection	the tcp connection structure.
 *  \param  buffer	        a buffer where the message will be stored.
 *  \param  len		        the length of the specified buffer.
 *  \return the number of bytes read.
 */
int tcpReceiveMessage(TCPConnection_t *udpConnection, char *buffer, int len);


/*! \brief Sends a TCP message.
 *
 *  Sends the specified TCP message.
 *
 *  \param  tcpConnection	the tcp connection structure.
 *  \param  buffer	        a buffer containing the message.
 *  \param  len		        the length of the message.
 *  \return the number of bytes sent.
 */
int tcpSendMessage(TCPConnection_t *udpConnection, const char *buffer, int len);


/*! \brief Closes a TCP connection.
 *
 *  Closes the specified TCP connection.
 *
 *  \param  tcpConnection	the tcp connection structure.
 *  \return the file descriptor for communicating with the client.
 */
void tcpCloseConnection(TCPConnection_t *udpConnection);


/*! \brief Terminates the TCP socket.
 *
 *  Frees the information associated with an TCP socket and closes it.
 *
 *  \param  tcpConnection	the tcp connection structure.
 */
void tcpDestroySocket(TCPConnection_t *udpConnection);



#endif  /* TCP_H */
