#ifndef PD_API_H
#define PD_API_H

#include "../common.h"
#include "../udp.h"


/* The information to allow communication with the autentication server. */
typedef struct connection_info_t {

	char pdip[IP_SIZE + 1];		// the ip address of the program.
	char pdport[PORT_SIZE + 1];		// the port of the program.
	char asip[IP_SIZE + 1];	 	// the ip address of the autentication server.
	char asport[PORT_SIZE + 1];		// the port of the autentication server.

} connectionInfo_t;



/* The user's information. */
typedef struct user_info_t {

	char *uid;				// the user's ID.
	char *pass;				// the user's password.
	bool_t connected;		// the connection flag.

} userInfo_t;



/*! \brief TODO
 *
 *  
 *  \param  udpConnection	the udp connection structure.
 * 	\return TRUE if succeds on sending error , FALSE otherwise.
 */
bool_t req_serverError(UDPConnection_t *asConnection);


/*! \brief Registers a user on the authentication system.
 *
 *  Validates the user's ID and passwords, sends the register request to the as
 *  server and, if succeds, fills the userInfo structure with the new value.
 * 
 *  \param  asConnection	the udp connection structure.
 * 	\param  connectionInfo	the senders' and receivers' IP and port.
 * 	\param  uid   			the new user's ID.
 * 	\param  pass			the new user's password.
 * 	\param  userInfo		the structure to store the user information (if the authentication succeds).
 * 	\return TRUE if the authentication succeds, FALSE otherwise.
 */
bool_t req_registerUser(UDPConnection_t *asConnection, const connectionInfo_t *connectionInfo, const char *uid, 
const char *pass, userInfo_t *userInfo);


/*! \brief Unregisters a user from the authentication system.
 *
 *  Sends a unregister requiest to the AS with the userInfo's parameters.
 * 
 *  \param  asConnection	the udp connection structure.
 * 	\param  userInfo		the structure to store the user information (if the authentication succeds).
 * 	\return TRUE if the unregister succeds, FALSE otherwise.
 */
bool_t req_unregisterUser(UDPConnection_t *asConnection, userInfo_t *userInfo);


/*! \brief Processes the server's requests to display the 2FA validation code.
 *
 *  Receives the uid, validation code and file operation and displays to the user.
 *  After that, sends a response back to the server.
 * 
 *  \param  asConnection	the udp connection structure.
 * 	\param  args		    the arguments of the request.
 * 	\param  userInfo		the structure to store the user information (if the authentication succeds).
 * 	\return TRUE if the validation code was received and displayed correctly, FALSE otherwise.
 */
bool_t req_valCode(UDPConnection_t *asConnection, char *args, userInfo_t *userInfo);


/*! \brief Checks the response from the server to the register request.
 *
 *  Verifies the response of the server and updates the userInfo if required.
 * 
 *	\param  status		the buffer with the status of the termination
 * 	\param  userInfo		the structure to store the user information (if the authentication succeds).
 * 	\return TRUE if the registration succed on the server, FALSE otherwise.
 */
bool_t resp_registerUser(char *status, userInfo_t *userInfo);


/*! \brief Checks the response from the server to the unregister request
 *
 *  Verifies the response of the server and updates the userInfo if required.
 * 
 * 	\param  status		the buffer with the status of the termination
 * 	\param  userInfo		the structure to store the user information (if the authentication succeds).
 * 	\return TRUE if the registration succed on the server, FALSE otherwise.
 */
bool_t resp_unregisterUser(char *status, userInfo_t *userInfo);



/*! \brief Resends the last message sent
 *
 *  Sends the last message sent (stored on the static global buffer) to the server.
 * 
 *  \param  asConnection	the udp connection structure.
 * 	\return TRUE if the message was properly sent, FALSE otherwise.
 */
bool_t req_resendLastMessage(UDPConnection_t *asConnection);


#endif  /* PD_AUX_H */