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


#define INIT_BUF(buf) buf[0] = '\0'; buf[BUFFER_SIZE-1] = '\0'
#define USER_CLEAR(node) node->uid[0] = '\0'
#define TID_CLEAR(node) node->tid = -1
#define IS_TID_CLEAR(node) node->tid == -1
#define RID_CLEAR(node) node->rid = -1
#define IS_RID_CLEAR(node) node->rid == -1
#define VC_CLEAR(node) node->vc = -1
#define IS_VC_CLEAR(node) node->vc == -1
#define FNAME_CLEAR(node) node->fname[0] = '\0'
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
void _cleanQueueUID(char *uid);
void _addMsgToQueue(char* uid, char* msg);
bool_t inUserList(char* uid);
userNode_t* _getUserNodeUID(char* uid);
void _cleanLogFile(char* dir_name, const char* fileType);
void cleanLogs(DIR* dir);

// PD
void resendMessagePD(UDPConnection_t *udpConn, pdNode_t *node);
bool_t req_registerPD(UDPConnection_t *udpConnec, UDPConnection_t *receiver, char* buf);
bool_t req_unregisterPD(UDPConnection_t *udpConnec, UDPConnection_t *receiver, char* buf);
bool_t resp_validationCode(UDPConnection_t *udpConn, UDPConnection_t *receiver, char* buf);
bool_t req_authOP(UDPConnection_t *udpConn, UDPConnection_t *receiver, char* buf);
bool_t req_serverErrorUDP(UDPConnection_t *udpConnec, UDPConnection_t *recvConnoc, char *msgBuffer);
// User
bool_t req_loginUser(userNode_t *nodeTCP, char* buf);
bool_t unregisterUser(userNode_t *nodeTCP);
bool_t req_fileOP(userNode_t *nodeTCP, UDPConnection_t *udpConnec, char* buf);
bool_t req_serverErrorTCP(TCPConnection_t *tcpConnect, char *msgBuffer);
bool_t resp_fileOP(char* uid, char* status);
bool_t req_auth(userNode_t *nodeTCP, char* buf);

void _loginUser(char* relative_path, char* dirname, char* filename, char* ip, int port);
void _registerPD(char* relative_path, char* dirname, char* filename, char* pdip, char* pdport);
void _storePassPD(char* relative_path, char* dirname, char* filename, char* pass);
bool_t _getUDPConnPD(char *uid, UDPConnection_t *conn);

#endif /* AS_AUX_H */