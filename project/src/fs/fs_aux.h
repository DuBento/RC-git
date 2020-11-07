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


#endif 	/* FS_AUX */