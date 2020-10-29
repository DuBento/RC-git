#ifndef USER_AUX_H
#define USER_AUX_H

#include "../common.h"
#include "../tcp.h"


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

} userInfo_t;




#endif 	/* USER_AUX */