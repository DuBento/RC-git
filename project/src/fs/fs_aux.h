#ifndef FS_AUX_H
#define FS_AUX_H

#include "../common.h"
#include "../udp.h"
#include "../tcp.h"

extern bool_t verbosity;

typedef struct connectionInfo_t {

	char fsip[IP_SIZE + 1];				// the ip address of the file server
	char fsport[PORT_SIZE + 1];			// the port of the file server
	char asip[IP_SIZE + 1];             // the IP address of authentication server
	char asport[PORT_SIZE + 1];         // the port of authentication server

} connectionInfo_t;

#endif 	/* FS_AUX */