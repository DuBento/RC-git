#ifndef TCP_H
#define TCP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "common.h"



/*! \brief Creates and initializes a TCP socket.
 *
 *  Creates a new socket for a TCP connection with the specified IP address
 *  and port number (after translating them into the appropriate formats).
 *
 * \param  addrIP	the ip address (IPv4 or IPv6).
 * \param  port		the service name (or the port number).
 * \return the socket's file descriptor.
 */
int tcpCreateSocket(const char *addrIP, const char *port);


/*! \brief Creates an TCP server.
 *
 *  Creates an TCP server with the specified IP address and port number.
 *
 * \param  addrIP	the ip address (IPv4 or IPv6).
 * \param  port		the service name (or the port number).
 * \param  nConnections	the maximum number of connections in queue.
 * \return the server's file descriptor.
 */
int tcpCreateServer(const char *addrIP, const char *port, int nConnections);


/*! \brief Creates an TCP client.
 *
 *  Creates an TCP client with the specified IP address and port number.
 *
 * \param  addrIP	the ip address (IPv4 or IPv6).
 * \param  port		the service name (or the port number).
 * \return the server's file descriptor.
 */
int tcpCreateClient(const char *addrIP, const char *port);


/*! \brief Connects the client with the server.
 *
 *  Tries to connect to the server.
 *
 * \param  fd		the TCP connection file descriptor.
 */
void tcpConnect(int fd);


/*! \brief Accepts the connections from the clients.
 *
 *  Accepts a new connection from a clients and returns a new file descriptor
 *  for communicating with it.
 *
 * \param  fd		the TCP connection file descriptor.
 * \return the file descriptor for communicating with the client.
 */
int tcpAcceptConnection(int fd);


/*! \brief Receives a TCP message.
 *
 *  Stores a TCP message in the specified buffer.
 *
 * \param  fd		the TCP connection file description.
 * \param  buffer	a buffer where the message will be stored.
 * \param  len		the length of the specified buffer.
 * \return the number of bytes read.
 */
int tcpReceiveMessage(int fd, char *buffer, int len);


/*! \brief Sends a TCP message.
 *
 *  Sends the specified TCP message.
 *
 * \param  fd		the TCP connection file description.
 * \param  buffer	a buffer containing the message.
 * \param  len		the length of the message.
 * \return the number of bytes sent.
 */
int tcpSendMessage(int fd, const char *buffer, int len);


/*! \brief Closes a TCP connection.
 *
 *  Closes the specified TCP connection.
 *
 * \param  fd		the TCP connection file descriptor.
 * \return the file descriptor for communicating with the client.
 */
void tcpCloseConnection(int fd);


/*! \brief Terminates the TCP socket.
 *
 *  Frees the information associated with an TCP socket and closes it.
 *
 * \param fd		the TCP connection file description.
 */
void tcpDestroySocket(int fd);



#endif  /* TCP_H */
