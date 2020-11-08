#ifndef AS_AUX_H
#define AS_AUX_H

#include "../common.h"
#include "../udp.h"
#include "../tcp.h"
#include "../files.h"

#define DIR_NAME	"USERS"
#define USERDIR_PREFIX 	"UID"
#define REGFILE_SUFIX   "_reg.txt"
#define PASSFILE_SUFIX  "_pass.txt"
#define LOGINFILE_SUFIX "_login.txt"
#define TIDFILE_SUFIX   "_tid.txt"
#define FILE_SIZE       32

typedef struct as_node_tcp {
	TCPConnection_t tcpConn;
	char uid[UID_SIZE+1];
}asNodeTCP_t;


/* typedef struct as_node_udp {
	struct as_node_udp *next;
	char uid[UID_SIZE+1];
	bool_t msg_sent;
	char msg[BUFFER_SIZE];
	UDPConnection_t udp_node;
} asNodeUDP_t; */

// UDP
bool_t req_registerPD(UDPConnection_t *udpConnec, UDPConnection_t *receiver, char* buf, char* path);
bool_t req_unregisterPD(UDPConnection_t *udpConnec, UDPConnection_t *receiver, char* buf, char* path);
bool_t req_serverErrorUDP(UDPConnection_t *udpConnec, UDPConnection_t *recvConnoc, char *msgBuffer);

// TCP
bool_t req_loginUser(asNodeTCP_t *nodeTCP, char* buf, char* path);
bool_t unregisterUser(asNodeTCP_t *nodeTCP, char* path);
bool_t req_serverErrorTCP(TCPConnection_t *tcpConnect, char *msgBuffer);


void _loginUser(char* relative_path, char* dirname, char* filename, char* ip, int port);
void _registerPD(char* relative_path, char* dirname, char* filename, char* pdip, char* pdport);
void _storePassPD(char* relative_path, char* dirname, char* filename, char* pass);

#endif /* AS_AUX_H */