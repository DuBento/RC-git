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

} userInfo_t;

// tejo: IP=193.136.138.142). AS  (TCP/UDP) no porto 58011; FS TCP no porto 59000.
//static connectionInfo_t connectionInfo = {TEJO_IP, TEJO_AS_PORT, TEJO_IP, TEJO_FS_PORT};

// Sigma testing fs
static connectionInfo_t connectionInfo = {TEJO_IP, TEJO_AS_PORT, "193.136.128.108\0", "59053\0"};

// Sigma testing as
//static connectionInfo_t connectionInfo = {"79.169.11.135\0", "5000\0", TEJO_IP, TEJO_FS_PORT};



static userInfo_t userInfo = { 0 };





#define RAND_NUM_MIN 1000
#define RAND_NUM_MAX 9999

#define RID_INVALID	0
#define	TID_INVALID	0

#define SSCANF_FAILURE	EOF

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

#define PROTOCOL_MSSG_OFFSET	4	// space between response letters and relevant data


/*! \brief Request user login.
 *
 *  Attempts to login user in Authentication Server (AS).
 *
 * \param  asConnection pointer to TCP AS connection structure.
 * \param  userInfo 	pointer to store the user's information.
 * \param  uid 		the user's UID.
 * \param  pass 	the user's password.
 * \return TRUE if login request is successeful; FALSE otherwise.
 */
bool_t req_login(TCPConnection_t *asConnection, userInfo_t *userInfo, const char *uid, const char *pass);


/*! \brief Request user request.
 *
 *  Attempts to make a user request near Authentication Server (AS).
 *
 * \param  asConnection pointer to TCP AS connection structure.
 * \param  userInfo 	pointer to store the user's information.
 * \param  fop		the file operation
 * \param  fname	the file name
 * \return request ID (RID).
 */
int req_request(TCPConnection_t *asConnection, const userInfo_t *userInfo, const char *fop, const char *fname);



/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_val(TCPConnection_t *asConnection, const userInfo_t *userInfo, const char *vc, int rid);



/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_list(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_retrieve(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid, const char *fname);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_upload(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid, const char *filename);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_delete(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid, const char *filename);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_remove(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_resendLastMessage();


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_login(char *status);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_request(char *status);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
int resp_val(char *tid);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_list(TCPConnection_t **fsConnection, char *data);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_retrieve(TCPConnection_t **fsConnection, char *status);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_upload(TCPConnection_t **fsConnection, char *status);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_delete(TCPConnection_t **fsConnection, char *status);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_remove(TCPConnection_t **fsConnection, char *status);


#endif 	/* USER_AUX */