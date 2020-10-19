#ifndef UDP_H
#define UDP_H

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "common.h"



/*! \brief Creates and initializes an UDP socket.
 *
 *  Creates a new socket for an UDP connection with the specified IP address
 *  and port number (after translating them into the appropriate formats).
 *
 * \param  addrIP   the ip address (IPv4 or IPv6).
 * \param  port     the service name (or the port number).
 * \return the socket's file descriptor.
 */
int udpCreateSocket(const char *addrIP, const char *port);


/*! \brief Creates an UDP server.
 *
 *  Creates an UDP server with the specified IP address and port number.
 *
 * \param  addrIP   the ip address (IPv4 or IPv6).
 * \param  port     the service name (or the port number).
 * \return the server's file descriptor.
 */
int udpCreateServer(const char *addrIP, const char *port);


/*! \brief Creates an UDP client.
 *
 *  Creates an UDP client with the specified IP address and port number.
 *
 * \param  addrIP   the ip address (IPv4 or IPv6).
 * \param  port     the service name (or the port number).
 * \return the server's file descriptor.
 */
int udpCreateClient(const char *addrIP, const char *port);


/*! \brief Receives an UDP message.
 *
 *  Stores an UDP message in the specified buffer.
 *
 * \param fd        the UDP connection file description.
 * \param buffer    a buffer where the message will be stored.
 * \param len       the length of the specified buffer.
 * \return the number of bytes read.
 */
int udpReceiveMessage(int fd, char *buffer, int len);



/*! \brief Sends an UDP message.
 *
 *  Sends the specified UDP message.
 *
 * \param fd        the UDP connection file description.
 * \param buffer    a buffer containing the message.
 * \param len       the length of the message.
 * \return the number of bytes sent.
 */
int udpSendMessage(int fd, const char *buffer, int len);


/*! \brief Terminates the UDP socket.
 *
 *  Frees the information associated with an UDP socket and closes it.
 *
 * \param fd        the UDP connection file description.
 */
void udpDestroySocket(int fd);



#endif /* UDP_H */