#ifndef USER_AUX_H
#define USER_AUX_H

#include "../common.h"
#include "../tcp.h"
#include <sys/select.h>
#include "../files.h"


/* the information to allow communication with the servers */
typedef struct connectionInfo_t {

	char asip[IP_SIZE + 1];         // the ip address of the autentication server.
	char asport[PORT_SIZE + 1];     // the port of the autentication server.       
	char fsip[IP_SIZE + 1];         // the ip address of the file system.
	char fsport[PORT_SIZE + 1];     // the port of the file system.

} connectionInfo_t;



/* The user's information. The information is only stored in this strutcture once its validity is confirmed.  */
typedef struct user_info_t {

	char *uid;				// the user's ID.
	char *pass;				// the user's password.
	bool_t asConnected;		// TRUE if User is connected to AS.
	bool_t fsConnected;		// TRUE if User is connected to FS.
	bool_t loggedIn;		// TRUE if User has already logged in.

} userInfo_t;







#define RID_INVALID	0
#define	TID_INVALID	0

#define SSCANF_FAILURE	EOF
#define	SPRINTF_THRSHLD	0
#define	SEPARATOR_SIZE	1
#define	CMD_PRT_SIZE	3
#define PRT_TERM_CHAR	'\n'

/* User commands */
#define CMD_LOGIN	"login"

#define CMD_REQ 	"req"

#define CMD_VAL         "val"

#define CMD_LIST        "list"
#define CMD_LIST_S      "l"

#define CMD_RETRIEVE    "retrieve"
#define CMD_RETRIEVE_S  "r"

#define CMD_UPLOAD      "upload"
#define CMD_UPLOAD_S    "u"

#define CMD_DELETE      "delete"
#define CMD_DELETE_S    "d"

#define CMD_REMOVE      "remove"
#define CMD_REMOVE_S    "x"

#define CMD_EXIT        "exit"

/* Fops */
#define FOP_STR_L	"L"
#define FOP_STR_R	"R"
#define FOP_STR_U	"U"
#define FOP_STR_D	"D"
#define FOP_STR_X	"X"

#define PROTOCOL_MSSG_OFFSET	4 // space between response letters and relevant data

#define MSG_TRY_AGAIN	"Please try again."

#define MSG_AS		"Authentication Server (AS)"
#define	MSG_FS		"File Server (FS)"
#define MSG_PD		"Personal Device (PD)"
#define MSG_TID		"Transaction ID (TID)"
#define MSG_UID		"User ID (UID)"
#define	MSG_VC		"Validation Code (VC)"
#define	MSG_PASS	"Password"
#define	MSG_FNAME	"File name"
#define MSG_FOP		"File Operation (Fop)"

#define MSG_NOT_RESP	" not responding."
#define MSG_SORRY	"Sorry for the inconvenience."
#define MSG_SHUTDOWN	"Shutting down..."
#define	MSG_THANKS	"Thanks. :)"
#define MSG_RECONTACT	"Trying to recontact..."

#define	MSG_ERR_COM	"Error in communication with "
#define	MSG_ERR_INV_REQ	"Invalid request!"
#define	MSG_ERR_INV_CMD	"Invalid command!"
#define MSG_ERR_INV_FOP	"Invalid file operation (Fop)!"
#define MSG_ERR_INV_FMT "Request incorrectly formatted."
#define	MSG_OP_IGN	"Operation ignored."
#define MSG_DNE		" does not exist"
#define MSG_FILES_DNE	"There are no files"
#define MSG_MAXFILES	"maximum files (15)"

#define MSG_SUC_LOG	"Login successeful."
#define MSG_SUC_AUT	"Authentication successeful."
#define	MSG_SUC_REQ	"Request successefully made."
#define	MSG_SUC_REM	"Remotion successeful!"
#define	MSG_SUC_UPL	"Upload successeful."

#define MSG_FLD		"Failed to "
#define MSG_FLD_AUT	"Authentication near Authentication Server (AS) failed."
#define MSG_FLD_LOG_PSW	"Incorrect password."
#define MSG_FLD_LOG	"A successeful login hasn't been made before. Please login successefully before making requests."
#define	MSG_FLD_SSCANF	"Failed to prepare message to "
#define MSG_FLD_UID	"Your UID is incorrect."
#define MSG_FLD_CONTACT	" has failed to contact "
#define MSG_FLD_VLD 	" has failed to validate "

#define MSG_INC_FORMAT	" not in correct format."

#define MSG_HELP_CORRVC	"\t-> Have you written the correct VC?"
#define	MSG_HELP_DUPVC	"\t-> Have you already inserted this VC?"
#define MSG_HELP_REGPD 	"\t-> Have you successefully registered your Personal Device (PD)?"
#define	MSG_HELP_MSGPD	"\t-> Have you received any message in your Personal Device (PD)?"
#define	MSG_HELP_VLDUID	"\t-> Is your username valid?"
#define	MSG_HELP_VLDPSW "\t-> Is your password valid?"
#define MSG_HELP_VLDFOP "\t-> Have you written a valid file operation? (R, D, L, U, X)"
#define MSG_HELP_UPCASE	"\t-> Have you written a File Operation with uppercase?"
#define	MSG_HELP_FNAME	"\t-> Have you written file name, if needed?"
#define MSG_HELP_PRVLOG	"\t-> Have you logged in before?"
#define	MSG_HELP_NOFILE	"\t-> Have you ever uploaded a file successefully?"
#define MSG_HELP_PREVRQ	"\t-> Have you made a request before?"

#define CURRENT_DIR	"./"
#define	LST_TABLE_HDR	"#\tFile Name\t\tSize\n\n"

#define	TCP_FLD_RCV	-1

/*! \brief Sends a user message through a TCP connection.
 *
 *  Sends through TCP the message contained in the buffer specified as argument.
 *
 * \param  tcpConnection	the TCP connection through which the message is 
 * 				sent.
 * \param  msgBuffer		the buffer containing the message.
 * \param  msgSize		
 * \return TRUE if message successefully sent.
 */
bool_t sendUserMessage(TCPConnection_t *tcpConnection, char *msgBuffer, int msgSize);


/*! \brief Request user login.
 *
 *  Attempts to login user in Authentication Server (AS). The User application
 *  sends the AS server a message with the user’s ID UID and password pass for 
 *  validation. 
 *
 * \param  asConnection pointer to TCP AS connection structure.
 * \param  userInfo 	pointer to store the user's information.
 * \param  uid 		the user's UID.
 * \param  pass 	the user's password.
 * \return TRUE if login request is successeful; FALSE otherwise.
 */
bool_t req_login(TCPConnection_t *asConnection, userInfo_t *userInfo, 
		const char *uid, const char *pass);


/*! \brief Request user request.
 *
 *  Attempts to make a user request near Authentication Server (AS). User sends 
 *  a message to the AS requesting a transaction ID code (TID). 
 *  This request message includes the UID and the type of file operation desired
 *  (Fop), either list (L), retrieve (R), upload (U), delete (D) or remove (X), 
 *  and if appropriate (when Fop is R, U or D) also sends the Fname.
 *  Upon receipt of this message, the AS will send the VLC message to the PD.
 *
 * \param  asConnection pointer to TCP AS connection structure.
 * \param  userInfo 	pointer to store the user's information.
 * \param  fop		the file operation
 * \param  fname	the file name
 * \return request ID (RID).
 */
int req_request(TCPConnection_t *asConnection, const userInfo_t *userInfo, 
		const char *fop, const char *fname);


/*! \brief Requests a validation code.
 *
 *  After the user checking the VC on the PD, the User application sends this
 *  message to the AS with the UID and the VC, along with the request identifier
 *  RID, to complete the second factor authentication. 
 *  A recently generated VC will be accepted by the AS only once.
 *
 * \param  asConnection	the authentication server connection.
 * \param  userInfo	the user's info.
 * \param  vc		the validation code received on Personal Device (PD).
 * \param  rid		the request's id.
 * \return TRUE if message is successefully sent; FALSE otherwise.
 */
bool_t req_val(TCPConnection_t *asConnection, const userInfo_t *userInfo, const char *vc, int rid);



/*! \brief Request to list files at FS.
 *
 *  User establishes TCP session with FS
 *  asking for the list of files this user has previously
 *  uploaded to the server. 
 *  The message includes the UID, the TID and the type of file operation 
 *  desired (Fop). 
 *  The reply should be displayed as a numbered list of filenames and the 
 *  respective sizes.
 *
 * \param  fsConnection	the File Server connection.
 * \param  userInfo	the user's info.
 * \param  tid		the transaction's ID.
 * \return TRUE if user is connected to FS.
 */
bool_t req_list(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid);


/*! \brief Request to retrieve a file from FS.
 *
 *  Following the retrieve command, the User application opens a TCP
 *  connection with the FS server to retrieve the contents of the file with name
 *  Fname from the FS server. The user ID (UID) and transaction ID (TID) are
 *  also provided. Before replying, the FS sends a message to the AS to validate 
 *  the transaction (VLD).
 * 
 * \param  fsConnection	the File Server connection.
 * \param  userInfo	the user's info.
 * \param  tid		the transaction's ID.
 * \param  fname	the file's name
 * \param  filename	pointer to store the filename.
 * \return TRUE if user is connected to FS.
 */
bool_t req_retrieve(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid, const char *fname, char **filename);


/*! \brief Request to upload a file from FS.
 *
 *  Following the upload command, the User application opens a TCP connection
 *  with the FS server and uploads to it the contents of the selected file (data),
 *  with name Fname and size Fsize bytes. The user ID (UID) and transaction ID
 *  (TID) are also provided. Before replying, the FS sends a message to the AS to
 *  validate the transaction (VLD).
 *
 * \param  fsConnection	the File Server connection.
 * \param  userInfo	the user's info.
 * \param  tid		the transaction's ID.
 * \param  filename	the file's name
 * \return TRUE if user is connected to FS.
 */
bool_t req_upload(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid, const char *filename);


/*! \brief Request to delete a file from FS.
 *
 *  Following the delete command, the User application opens a TCP connection
 *  with the FS server and requests the deletion of the file with name Fname. 
 *  The user ID (UID) and transaction ID (TID) are also provided. 
 *  Before replying, the FS sends a message to the AS to validate the 
 *  transaction (VLD).
 *
 * \param  fsConnection	the File Server connection.
 * \param  userInfo	the user's info.
 * \param  tid		the transaction's ID.
 * \param  filename	the file's name
 * \return TRUE if user is connected to FS.
 */
bool_t req_delete(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid, const char *filename);


/*! \brief Request to remove information from both servers.
 *
 *  Following the remove command, the User application opens a TCP connection
 *  with the FS server and requests the removal of all its files and directories 
 *  from the FS server, as well as the deletion of the user information from the
 *  AS server.
 *  The user ID (UID) and transaction ID (TID) are also provided. Before replying,
 *  the FS sends a message to the AS to validate the transaction (VLD) and
 *  requesting the AS to remove the user information.
 *
 * \param  fsConnection	the File Server connection.
 * \param  userInfo	the user's info.
 * \param  tid		the transaction's ID.
 * \return TRUE if user is connected to FS.
 */
bool_t req_remove(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid);


/*! \brief Resends last user message.
 *
 *  Attempts to resend the user's message because a response was not received in
 *  time.
 *
 * \return TRUE if the attempt is successeful; FALSE otherwise.
 */
bool_t req_resendLastMessage();


/*! \brief Receives response from login request.
 *
 *  In reply to a LOG request the AS server replies with the status of the login
 *  request. 
 *  If UID and pass are valid the status is OK; 
 *  if the UID exists but the pass is incorrect the status is NOK; 
 *  otherwise the status is ERR.
 *
 * \param  userInfo	pointer to structure where user info is stored.
 * \return TRUE if login was successefull; FALSE otherwise.
 */
bool_t resp_login(userInfo_t *userInfo, char *status);


/*! \brief Receives response from request request.
 *
 *  The AS server replies informing if the REQ request could be processed (valid
 *  UID), a message was sent to the PD and a successful RVC confirmation received.
 *  In case of success the status is OK; 
 *  if the REQ request was sent in a TCP connection where a successful login was
 *  not previously done,
 *  the status is ELOG; 
 *  if a message could not be sent by the AS to the PD the status is EPD;
 *  if the UID is incorrect the status is EUSER; 
 *  if the Fop is invalid the status is EFOP; 
 *  otherwise (e.g. incorrectly formatted REQ message) the status is ERR.
 *
 * \param  status	response from Authentication Server (AS).
 * \return TRUE.
 */
bool_t resp_request(char *status);


/*! \brief Receives response from val request.
 *
 *  The AS confirms (or not) the success of the two-factor authentication, 
 *  sending the transaction identifier TID to use in the file operation with the
 *  FS. The TID takes value 0 if the authentication failed.
 *
 * \param  tidStr	the transaction id received.
 * \return the Transaction ID (TID).
 */
int resp_val(char *tidStr);


/*! \brief Receives response from list request.
 *
 *  After receiving a message from the AS validating the transaction (CNF), the FS
 *  reply to a User application LST request contains the number N of available files,
 *  and for each file:
 *  • the filename Fname, limited to a total of 24 alphanumerical characters (plus
 *  ‘-‘, ‘_’ and ‘.’), including the separating dot and the 3-letter extension:
 *  “nnn…nnnn.xxx”;
 *  • the file size Fsize, in bytes.
 *  The filenames should be displayed by the User application as a numbered list.
 *  In case of error the reply is in the form RLS status, with 
 *  RLS N[ Fname Fsize]
 *  RLS status:
 *  - status = EOF if no files are available, 
 *  - status = INV in case of an AS validation error of the provided TID,
 *  - status = ERR if the LST request is not correctly formulated	
 *
 * \param  fsConnection	the FS connection.
 * \param  data		the response + list of files given by FS.	
 * \return TRUE if user is no longer connected to FS.
 */
bool_t resp_list(TCPConnection_t **fsConnection, char *data);


/*! \brief Receives response from retrieve request.
 *
 *  After receiving a message from the AS validating the transaction (CNF), and 
 *  in reply to a RTV request, the FS server transfers to the User application 
 *  the contents (data) of the selected file, as well as the file size Fsize in 
 *  bytes. 
 *  If the RTV request was successful the status is OK, 
 *  the status is EOF if the file is not available, 
 *  the status is NOK if there is no content available in the FS
 *  for the user with ID UID, 
 *  the status is INV in case of an AS validation error
 *  of the provided TID, and 
 *  the status is ERR if the RTV request is not correctly
 *  formulated.
 *  The name and path where the file is stored are displayed by the User 
 *  application.
 *  After receiving the reply message, the User application closes the TCP
 *  connection with the FS.
 *
 * \param  fsConnection	the FS connection.
 * \param  status	the status received by the user
 * \param  filename	the file's name.
 * \param  tcpMsgSize	the size of the message already read.
 * \return TRUE if user is no longer connected to FS.
 */
bool_t resp_retrieve(TCPConnection_t **fsConnection, char *status, 
	char **filename, int tcpMsgSize);


/*! \brief Receives response from upload request.
 *
 *  After receiving a message from the AS (CNF) validating the transaction, the
 *  answer to a UPL request consists in the FS server replying with the status 
 *  of the file transfer. 
 *  If the UPL request was successful the status is OK, the status
 *  is NOK if the UID does not exist, the status is DUP if the file already 
 *  existed,
 *  the status is FULL if 15 files were previously uploaded by this User, the
 *  status is INV in case of an AS validation error of the provided TID, and the
 *  status is ERR if the UPL request is not correctly formulated.
 *  The upload success (or not) is displayed by the User application.
 *  After receiving the reply message, the User application closes the TCP
 *  connection with the FS.
 *
 * \param  fsConnection	the FS connection.
 * \param  status	the status received by the user
 * \return TRUE if user is no longer connected to FS.
 */
bool_t resp_upload(TCPConnection_t **fsConnection, char *status);


/*! \brief Receives response from delete request.
 *
 *  After receiving a message from the AS (CNF) validating the transaction, the
 *  answer to a DEL request consists in the FS server replying with the status 
 *  of the file deletion. 
 *  If the DEL request was successful the status is OK, 
 *  the status is EOF if the file is not available, 
 *  the status is NOK if the UID does not exist,
 *  the status is INV in case of an AS validation error of the provided TID, and
 *  the status is ERR if the DEL request is not correctly formulated.
 *  The delete success (or not) is displayed by the User application.
 *  After receiving the reply message, the User application closes the TCP
 *  connection with the FS.
 *
 * \param  fsConnection	the FS connection.
 * \param  status	the status received by the user
 * \return TRUE if user is no longer connected to FS.
 */
bool_t resp_delete(TCPConnection_t **fsConnection, char *status);


/*! \brief Receives response from remove request.
 *
 *  After receiving a message from the AS (CNF) validating the transaction and
 *  confirming the user deletion in the AS, the FS removes all the user’s files and
 *  directories. 
 *  It then replies with the status of the operation: 
 *  the status is OK if the REM request was successful, 
 *  the status is NOK if the UID does not exist,
 *  the status is INV in case of an AS validation error of the provided TID, and
 *  the status is ERR if the REM request is not correctly formulated.
 *  The remove success (or not) is displayed by the User application.
 *  After receiving the reply message, the User application closes the TCP
 *  connection with the FS.
 *
 * \param  fsConnection	the FS connection.
 * \param  userInfo	the user's info.
 * \param  status	the status received by the user
 * \return TRUE if user is no longer connected to FS.
 */
bool_t resp_remove(TCPConnection_t **fsConnection, const userInfo_t *userInfo, 
	char *status);


#endif 	/* USER_AUX */