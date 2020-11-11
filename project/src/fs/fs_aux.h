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
typedef struct userRequest_t {
	TCPConnection_t *tcpConnection;
	float timeExpired;
	int nTries;
	char fop;
	void(*exeRequest)(struct userRequest_t*, const char*);

	char replyHeader[4];
	int fsid;

	char uid[UID_SIZE + 1];
	char tid[TID_SIZE + 1];
	char *fileName;

} userRequest_t;





/*! \brief Prepares a list request from the user.
 *
 *  Checks if a user requested a list of files and fills the request accordingly
 * 	(if all the parameters are correct).
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param 	uid				the uid of the request.
 * 	\param	tid				the tid of the request.
 * 	\return TRUE if the fill was successfull, FALSE otherwise.
 */
bool_t fillListRequest(userRequest_t *userRequest, const char* uid, const char *tid);


/*! \brief Prepares a retreive request from the user.
 *
 *  Checks if a user requested a retreive of a file and fills the request accordingly
 * 	(if all the parameters are correct).
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param 	uid				the uid of the request.
 * 	\param	tid				the tid of the request.
 *  \param  fname			the name of the file.
 * 	\return TRUE if the fill was successfull, FALSE otherwise.
 */
bool_t fillRetreiveRequest(userRequest_t *userRequest, const char* uid, const char *tid, const char *fname);


/*! \brief Prepares a upload request from the user.
 *
 *  Checks if a user requested an upload of a file and fills the request accordingly
 * 	(if all the parameters are correct).
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param 	uid				the uid of the request.
 * 	\param	tid				the tid of the request.
 *  \param  fname			the name of the file.
 * 	\param	fsize			the file of the contents.
 * 	\param	fdata			the contents of the firt buffer.
 * 	\return TRUE if the fill was successfull, FALSE otherwise.
 */
bool_t fillUploadRequest(userRequest_t *userRequest, const char* uid, const char *tid, const char *fname, const char *fdata);	


/*! \brief Prepares a delete request from the user.
 *
 *  Checks if a user requested a delete of a file and fills the request accordingly
 * 	(if all the parameters are correct).
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param 	uid				the uid of the request.
 * 	\param	tid				the tid of the request.
 *  \param  fname			the name of the file.
 * 	\return TRUE if the fill was successfull, FALSE otherwise.
 */
bool_t fillDeleteRequest(userRequest_t *userRequest, const char* uid, const char *tid, const char *fname);


/*! \brief Prepares a remove request from the user.
 *
 *  Checks if a user requested to remove all files and fills the request accordingly
 * 	(if all the parameters are correct).
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param 	uid				the uid of the request.
 * 	\param	tid				the tid of the request.
 * 	\return TRUE if the fill was successfull, FALSE otherwise.
 */
bool_t fillRemoveRequest(userRequest_t *userRequest, const char* uid, const char *tid);





/*! \brief Executes the list request.
 *
 *  Gathers all files in the user's directory and sends back a list with their names.
 * 
 * 	\param  userRequest		the user's request.
 * 	\param  filesPath		the path to the files root directory.
 */
void listRequest(userRequest_t *userRequest, const char *filesPath);


/*! \brief Executes the retreive request.
 *
 *  Reads a file and send back to the user its contents.
 * 
 * 	\param  userRequest		the user's request.
 * 	\param  filesPath		the path to the files root directory.
 */
void retreiveRequest(userRequest_t *userRequest, const char *filesPath);


/*! \brief Executes the upload request.
 *
 *  Stores a new file in the users directory.
 * 
 * 	\param  userRequest		the user's request.
 * 	\param  filesPath		the path to the files root directory.
 */
void uploadRequest(userRequest_t *userRequest, const char *filesPath);


/*! \brief Executes the delete request.
 *
 *  Removes a file from the user's directory.
 * 
 * 	\param  userRequest		the user's request.
 * 	\param  filesPath		the path to the files root directory.
 */
void deleteRequest(userRequest_t *userRequest, const char *filesPath);


/*! \brief Executes the remove request.
 *
 *  Deletes the entire directory of the user.
 * 
 * 	\param  userRequest		the user's request.
 * 	\param  filesPath		the path to the files root directory.
 */
void removeRequest(userRequest_t *userRequest, const char *filesPath);





/*!	\brief Sends a validation request to the AS server.
 *
 * 	Sends a validation request to the AS server.
 * 
 *  \param 	asServer			the UDP connection with the AS server.
 * 	\param 	userRequest			the request of the user to be validated.
 */
void validateRequest(UDPConnection_t *asServer, userRequest_t *userRequest);



#endif 	/* FS_AUX */