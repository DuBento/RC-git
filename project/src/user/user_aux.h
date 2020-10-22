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



#endif 	/* USER_AUX */