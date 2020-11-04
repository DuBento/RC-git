#ifndef USER_AUX_H
#define USER_AUX_H

#include "../common.h"
#include "../tcp.h"
#include <sys/select.h>


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
	bool_t connected;		// the connection flag.
	bool_t fsConnected;		// TRUE if User is connected to FS

} userInfo_t;

#define RAND_NUM_MIN 1000
#define RAND_NUM_MAX 9999


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

/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_login(TCPConnection_t *asConnection, userInfo_t *userInfo, const char *uid, const char *pass);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_request(TCPConnection_t *asConnection, const userInfo_t *userInfo, const char *fop, const char *fname, int *rid);



/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_val(TCPConnection_t *asConnection, const userInfo_t *userInfo, const char *vc, int *rid);



/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_list();


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_retrieve();


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_upload();


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_delete();


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_remove();


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t req_exit();


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
bool_t resp_val(char *tid);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_list();


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_retrieve();


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_upload(TCPConnection_t *fsConnection, char *status);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_delete(TCPConnection_t *fsConnection, char *status);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_remove(TCPConnection_t *fsConnection, char *status);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
bool_t resp_exit();

#endif 	/* USER_AUX */