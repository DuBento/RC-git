#ifndef TCP_H
#define TCP_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "common.h"

#define CLIENT  'c'
#define SERVER  's'


/* Structure that stores the information for the TCP connection. */
typedef struct tcp_connection {
    int fd;
    struct sockaddr addr;
    socklen_t addrlen;
} TCPConnection_t;



/*! \brief Creates and initializes a TCP socket.
 *
 *  Creates a new socket for a TCP connection with the specified IP address
 *  and port number (after translating them into the appropriate formats).
 *
 *  \param  addrIP  	the ip address (IPv4 or IPv6).
 *  \param  port	    the service name (or the port number).
 *  \param  mode        CLIENT or SERVER (different specifications - check def)
 *  \return the tcp connection structure.
 */
TCPConnection_t* tcpCreateSocket(const char *addrIP, const char *port, char mode);


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
bool_t tcpConnect(TCPConnection_t *tcpConnection);


/*! \brief Accepts the connections from the clients.
 *
 *  Accepts a new connection from a clients and returns a new file descriptor
 *  for communicating with it.
 *
 *  \param  tcpConnection	the tcp connection structure.
 *  \return the file descriptor for communicating with the client.
 */
int tcpAcceptConnection(TCPConnection_t *tcpConnection, TCPConnection_t *newCon);


/*! \brief Receives a TCP message.
 *
 *  Stores a TCP message in the specified buffer.
 *
 *  \param  tcpConnection   socket from which the message will be received.
 *  \param  buffer	        a buffer where the message will be stored.
 *  \param  len		        the length of the specified buffer.
 *  \return the number of bytes read.
 */
int tcpReceiveMessage(TCPConnection_t *tcpConnection, char *buffer, int len);


/*! \brief Receives a TCP fixed size message.
 *
 *  Stores a TCP message in the specified buffer. This function will terminate once
 *  len bytes are read or the timeout is reached.
 *  \param  tcpConnection   socket from which the message will be received.
 *  \param  buffer	        a buffer where the message will be stored.
 *  \param  len		        the length of the specified buffer.
 *  \return the number of bytes read.
 */
int tcpReceiveFixedMessage(TCPConnection_t *tcpConnection, char *buffer, int len);



/*! \brief Sends a TCP message.
 *
 *  Sends the specified TCP message.
 *
 *  \param  tcpConnection   socket to which the message will be sent.
 *  \param  buffer	        a buffer containing the message.
 *  \param  len		        the length of the message.
 *  \return the number of bytes sent.
 */
int tcpSendMessage(TCPConnection_t *tcpConnection, const char *buffer, int len);


/*! \brief Closes a TCP connection.
 *
 *  Closes the specified TCP connection.
 *
 *  \param  tcpConnection	the tcp connection structure.
 *  \return the file descriptor for communicating with the client.
 */
void tcpCloseConnection(TCPConnection_t *tcpConnection);






void tcpCloseConnection_noAlloc(TCPConnection_t tcpConnection);


/*! \brief Terminates the TCP socket.
 *
 *  Frees the information associated with an TCP socket and closes it.
 *
 *  \param  tcpConnection	the tcp connection structure.
 */
TCPConnection_t *tcpDestroySocket(TCPConnection_t *tcpConnection);


/*! \brief Returns the IP address associated with a TCP connection.
 *
 *  Returns a string with the IP address of the specified connection.
 *
 *  \param  conn        the TCP connection.
 *  \return the IP address of the tcp connection.
 */
char* tcpConnIp(TCPConnection_t *conn);


/*! \brief Returns the port associated with a TCP connection.
 *
 *  Returns a string with the port of the specified connection.
 *
 *  \param  conn        the TCP connection.
 *  \return the port of the tcp connection.
 */
int tcpConnPort(TCPConnection_t *conn);


#endif  /* TCP_H */
