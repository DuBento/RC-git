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


#define INIT_BUF(buf) buf[0] = '\0'; buf[BUFER_SIZE-1] = '\0';
#define USER_CLEAR(node) node->uid[0] = '\0'
#define USER_LOGEDIN(node) node->uid[0] != '\0'

typedef struct user_node {
	TCPConnection_t tcpConn;
	char uid[UID_SIZE+1];
	short vc;
	short rid;
	short tid;
	char fop;
	char fname[FILE_NAME_SIZE+1];
}userNode_t;



typedef struct pd_node {
	// bool_t msg_sent; // unnecessary because if in list, msg sent no response received 
	short nAttempts;
	char uid[UID_SIZE+1];
	char msg[BUFFER_SIZE];
} pdNode_t;

// remove and adds msgs from waitingReply Queue for specified uid
void _cleanQueueFromUID(List_t list, char *uid);
void _addMsgToQueue(List_t ist, char* uid, char* msg);
bool_t inUserList(List_t userList, char* uid);
userNode_t* _getUserNodeUID(List_t list, char* uid);
void _cleanLogFile(char* dir_name, const char* fileType);
void cleanLogs(DIR* dir, char* path);

// PD
void resendMessagePD(UDPConnection_t *udpConn, pdNode_t *node, char * path);
bool_t req_registerPD(UDPConnection_t *udpConnec, UDPConnection_t *receiver, char* buf, char* path);
bool_t req_unregisterPD(UDPConnection_t *udpConnec, UDPConnection_t *receiver, char* buf, char* path, List_t list);
bool_t resp_validationCode(UDPConnection_t *udpConn, UDPConnection_t *receiver, List_t userList, List_t pdList, char* buf);
bool_t req_authOP(UDPConnection_t *udpConn, UDPConnection_t *receiver, char* buf, List_t userlist);
bool_t req_serverErrorUDP(UDPConnection_t *udpConnec, UDPConnection_t *recvConnoc, char *msgBuffer);
// User
bool_t req_loginUser(userNode_t *nodeTCP, char* buf, char* path);
bool_t unregisterUser(userNode_t *nodeTCP, char* path, List_t list);
bool_t req_fileOP(userNode_t *nodeTCP, char* buf, char* path, UDPConnection_t *udpConnec, List_t list);
bool_t req_serverErrorTCP(TCPConnection_t *tcpConnect, char *msgBuffer);
bool_t resp_fileOP(List_t list, char* uid, char* status);
bool_t req_auth(userNode_t *nodeTCP, char* buf);

void _loginUser(char* relative_path, char* dirname, char* filename, char* ip, int port);
void _registerPD(char* relative_path, char* dirname, char* filename, char* pdip, char* pdport);
void _storePassPD(char* relative_path, char* dirname, char* filename, char* pass);
bool_t _getUDPConnPD(char *uid, char* path, UDPConnection_t *conn);

#endif /* AS_AUX_H */