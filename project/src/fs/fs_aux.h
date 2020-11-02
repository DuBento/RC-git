#ifndef FS_AUX_H
#define FS_AUX_H

#include "../common.h"

typedef struct connectionInfo_t {

	char fsport[PORT_SIZE + 1];          /* port of the file server */
	char asip[IP_SIZE + 1];              /* IP of authentication server*/
	char asport[PORT_SIZE + 1];          /* port of authentication server */

} connectionInfo_t;

#endif 	/* FS_AUX */