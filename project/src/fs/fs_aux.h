#ifndef FS_AUX_H
#define FS_AUX_H

#include "../common.h"
#include "../files.h"
#include "../list.h"
#include "../udp.h"
#include "../tcp.h"

extern bool_t verbosity;


// the structure that stores the information about the server connection
typedef struct connectionInfo_t {

	char fsport[PORT_SIZE + 1];			// the port of the file server
	char asip[IP_SIZE + 1];             // the IP address of authentication server
	char asport[PORT_SIZE + 1];         // the port of authentication server

} connectionInfo_t;


// the structure that stores a request from the user
typedef struct userRequest {
	TCPConnection_t *tcpConnection;
	float timeExpired;
	int nTries;
	char fop;
	void(*exeRequest)(void*);

	char uid[UID_SIZE + 1];
	char tid[TID_SIZE + 1];
	char *fileName;
	size_t fileSize;
	char *data;

} userRequest_t;



/*! \brief Prepares a list request from the user.
 *
 *  Checks if a user requested a list of files and fills the request accordingly
 * 	(if all the parameters are correct)
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param  opcode			the opcode of the request specified by the user.
 * 	\param 	uid				the uid of the user.
 * 	\param	tid				the tid of the request.
 * 	\return TRUE if the list request is valid, FALSE otherwise
 */
bool_t fillListRequest(userRequest_t *userRequest, const char *opcode, const char *uid, const char *tid);


/*! \brief Prepares a retreive request from the user.
 *
 *  Checks if a user requested a retreive of a file and fills the request accordingly
 * 	(if all the parameters are correct)
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param  opcode			the opcode of the request specified by the user.
 * 	\param 	uid				the uid of the user.
 * 	\param	tid				the tid of the request.
 *  \param  fname			the name of the file.
 * 	\return TRUE if the retreive request is valid, FALSE otherwise
 */
bool_t fillRetreiveRequest(userRequest_t *userRequest, const char *opcode, const char *uid, const char *tid, 
	const char *fname);


/*! \brief Prepares a upload request from the user.
 *
 *  Checks if a user requested an upload of a file and fills the request accordingly
 * 	(if all the parameters are correct)
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param  opcode			the opcode of the request specified by the user.
 * 	\param 	uid				the uid of the user.
 * 	\param	tid				the tid of the request.
 *  \param  fname			the name of the file.
 * 	\param	fsize			the file of the contents.
 * 	\param	fdata			the contents of the file.
 * 	\return TRUE if the retreive request is valid, FALSE otherwise
 */
bool_t fillUploadRequest(userRequest_t *userRequest, const char *opcode, const char *uid, const char *tid, 
	const char *fname, const char *fsize);


/*! \brief Prepares a delete request from the user.
 *
 *  Checks if a user requested a delete of a file and fills the request accordingly
 * 	(if all the parameters are correct)
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param  opcode			the opcode of the request specified by the user.
 * 	\param 	uid				the uid of the user.
 * 	\param	tid				the tid of the request.
 *  \param  fname			the name of the file.
 * 	\return TRUE if the retreive request is valid, FALSE otherwise
 */
bool_t fillDeleteRequest(userRequest_t *userRequest, const char *opcode, const char *uid, const char *tid, 
	const char *fname);


/*! \brief Prepares a remove request from the user.
 *
 *  Checks if a user requested to remove all files and fills the request accordingly
 * 	(if all the parameters are correct)
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param  opcode			the opcode of the request specified by the user.
 * 	\param 	uid				the uid of the user.
 * 	\param	tid				the tid of the request.
 * 	\return TRUE if the retreive request is valid, FALSE otherwise
 */
bool_t fillRemoveRequest(userRequest_t *userRequest, const char *opcode, const char *uid, const char *tid);


/*! \brief Sends an error to the user (invalid request).
 *
 *  Sends the ERR message back to the user.
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\return TRUE if the retreive request is valid, FALSE otherwise
 */
bool_t replyInvalidRequest(userRequest_t *userRequest);

#endif 	/* FS_AUX */