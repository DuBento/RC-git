#include "as_aux.h"
#include "../common.h"
#include "../udp.h"
#include "../tcp.h"
#include "../list.h"
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <linux/limits.h>

typedef struct connectionInfo_t {

	char asport[PORT_SIZE + 1];             /* port of the autentication server */

} connectionInfo_t;

/* ========== GLOBAL ============= */
int exitCode = INIT_RUNTIME;
DIR *dir = NULL;
char mainDir_path[PATH_MAX];
TCPConnection_t *tcpServer = NULL;
UDPConnection_t *udpServer = NULL;
List_t userList = NULL;
List_t pdList = NULL;
char msgBuffer[2*BUFFER_SIZE];	// prevent overflows, giving space to concatenate msgs
char verbosity = FALSE;


/* ======= */
/* General */
/* ======= */
void parseArgs(int argc, char *argv[], connectionInfo_t *info) {
	/* check the number of arguments */        
	if (argc < 1 || argc > 4)
		_FATAL("Failed to parse arguments.\nUsage: %s [-p ASport] [-v]\n", argv[0]);
	
	// else
	for (int i = 1; i < argc; i++){
		if (!strcmp(ARG_ASPORT, argv[i]) && isPortValid((const char*) argv[i+1])
			&& i+1 < argc)
			strncpy(info->asport, argv[++i], PORT_SIZE);
		else if (!strcmp(ARG_VERBOS, argv[i]))
			/* activate verbose mode - flag */
			verbosity = TRUE;
		else 
			FATAL("Invalid PORT format. \nPlease only use unrestricted ports.");
	}

	/* logs the server information (on debug mod only) */
	_LOG("Runtime settings:\nASport\t: %s\nVerbosity\t: %d", 
		info->asport, verbosity);
}



void cleanListNodeUser(void* nodeData) {
	userNode_t *nodeDataAS =  (userNode_t*) nodeData;
	tcpCloseConnection_noAlloc(nodeDataAS->tcpConn);
	free(nodeDataAS);
}

void immediateExitAS() {
	// no logs where made before runtime so no need to clear them
	if (udpServer != NULL) 	udpDestroySocket(udpServer);
	if (tcpServer != NULL) 	tcpDestroySocket(tcpServer);
	if (userList != NULL)	listDestroy(userList, cleanListNodeUser);
	if (pdList != NULL)		listDestroy(pdList, free);
	if (dir != NULL)		closedir(dir);
}

void exitAS(int flag) {
	udpDestroySocket(udpServer);
	tcpDestroySocket(tcpServer);
	cleanLogs(dir);
	listDestroy(userList, cleanListNodeUser);
	listDestroy(pdList, free);
	closedir(dir);
	exit(flag);
}



/*! \brief Set program to terminate on success.
 *
 *	Termination handle called by the SIGINT and SIGTERM signals.
 */
void terminateAS() {
	if (exitCode == INIT_RUNTIME) {
		immediateExitAS();
		exit(EXIT_SUCCESS);
	}
	exitCode = EXIT_SUCCESS;
}


/*! \brief Set program to terminate on fatal errors.
 *
 *	Termination handle called by the SIGABRT, SIGFPE, SIGILL and SIGSEGV signals
 */
void abortAS() {
	if (exitCode == INIT_RUNTIME) {
		immediateExitAS();
		exit(EXIT_FAILURE);
	}
	exitCode = EXIT_FAILURE;
}


/* Handle UDP Responses (Incoming Messages) */
bool_t handleUDP(UDPConnection_t *udpConnec, char *msgBuf) {
	int n;
	char opcode[BUFFER_SIZE];
	UDPConnection_t recvConnoc;
	n = udpReceiveMessage(udpConnec, &recvConnoc, msgBuf, BUFFER_SIZE);

	sscanf(msgBuf, "%s", opcode);

	// PD
	// Registration Request
	if (!strcmp(opcode, REQ_REG))
		req_registerPD(udpConnec, &recvConnoc, msgBuf+strlen(REQ_REG));
	
	// Unregistration Request
	else if (!strcmp(opcode, REQ_UNR))
		req_unregisterPD(udpConnec, &recvConnoc, msgBuf+strlen(REQ_UNR));                

	// Validation Code received "RVC"
	else if (!strcmp(opcode, RESP_VLC))
		resp_validationCode(udpConnec, &recvConnoc, msgBuf+strlen(REQ_UNR));
		
	// FS
	else if (!strcmp(opcode, REQ_VLD))
		req_authOP(udpConnec, &recvConnoc, msgBuf+strlen(REQ_VLD));
	// General
	else if (!strcmp(opcode, SERVER_ERR)) {
		WARN("Invalid request! Operation ignored.");
		return FALSE;
	}
	else{
		_WARN("Invalid opcode on the server response! Sending error... Got: %s", opcode);
		req_serverErrorUDP(udpConnec, &recvConnoc, msgBuf);
		return FALSE;
	}
}

bool_t handleTCP(userNode_t *tcpNode, char *msgBuf) {
	char opcode[BUFFER_SIZE];

	if (tcpReceiveMessage(&tcpNode->tcpConn, msgBuf, BUFFER_SIZE) == -1){
		unregisterUser(tcpNode);
		return FALSE;	
	}

	sscanf(msgBuf, "%s", opcode);

	// Login Request
	if (!strcmp(opcode, REQ_LOG))
		req_loginUser(tcpNode, msgBuf+strlen(REQ_LOG));
	
	// File manipulation Request
	else if (!strcmp(opcode, REQ_REQ))
		req_fileOP(tcpNode, udpServer, msgBuf+strlen(REQ_LOG));
	
	// Authorize Op
	else if (!strcmp(opcode, REQ_AUT))
		req_auth(tcpNode, msgBuf+strlen(REQ_AUT));
	
	// Error
	else if (!strcmp(opcode, SERVER_ERR)) {
		WARN("Invalid request! Operation ignored.");
	}
	else{
		_WARN("Invalid opcode on the server response! Sending error. Got: %s", opcode);
		req_serverErrorTCP(&tcpNode->tcpConn, msgBuf);
	}

	return TRUE;
}




void addSocket(TCPConnection_t *tcpConnec, fd_set *fds, int *fdsSize) {
	if (fds == NULL || fdsSize == NULL)	return;
	if (*fdsSize < tcpConnec->fd+1)	*fdsSize = tcpConnec->fd+1;
	FD_SET(tcpConnec->fd, fds);
}

void removeSocket(TCPConnection_t *tcpConnec, fd_set *fds, int *fdsSize) {
	if (fds == NULL || fdsSize == NULL)	return;
	int fd = tcpConnec->fd;
	if (*fdsSize == (fd + 1))	*fdsSize--;
	FD_CLR(fd, fds);
}

void waitMainEvent(TCPConnection_t *tcp_server, UDPConnection_t *udp_server, char *msgBuf) {
	fd_set fds, ready_fds;
	struct timeval tv, tmp_tv;
	int selectRet, fds_size;
	int nTry = 0;
	ListIterator_t iter;
	/* SELECT */
	FD_ZERO(&fds);
	FD_SET(tcp_server->fd, &fds);
	FD_SET(udp_server->fd, &fds);
	fds_size = (tcp_server->fd > udp_server->fd) ? tcp_server->fd+1 : udp_server->fd+1;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;
	
	exitCode = RUNTIME;			// tells signal handler how to behave
	while (exitCode != EXIT_FAILURE && exitCode != EXIT_SUCCESS) {
		// because select is destructive
		ready_fds = fds;
		tmp_tv = tv;

		selectRet = select(fds_size, &ready_fds, NULL, NULL, &tmp_tv);

		if (selectRet  == -1){
			if (errno == EINTR) break;	// return from signal
			_FATAL("Unable to start the select() to monitor the descriptors!\n\t - Error code: %d", errno);
		}
		
		// timeout expired
		iter = listIteratorCreate(pdList);
		if (selectRet == 0) {
			while (!listIteratorEmpty(&iter)){
				ListNode_t node = (ListNode_t) iter;
				pdNode_t *nodeData = listIteratorNext(&iter);
				// act as previous message didn't reach the target
				// try to resend NTRIES_NORESP times
				if (nodeData->nAttempts < NREQUEST_TRIES)
					resendMessagePD(udpServer, nodeData);	//handle no reponse to prev msg
				else{
					nTry = 0;
					listRemove(pdList, node, free);
					_WARN("No response received from sent message.\nUID:%s\nCommunication error.", nodeData->uid);
				}
			}
			continue;	// run select again
		}       

		// string before the server output
		putStr(STR_RESPONSE, TRUE);

		// handle PD interaction
		if (FD_ISSET(udp_server->fd , &ready_fds))
			handleUDP(udp_server, msgBuf);

		// handle User new connection
		if (FD_ISSET(tcp_server->fd, &ready_fds)){
			userNode_t *newNode = (userNode_t*) malloc(sizeof(userNode_t));
			tcpAcceptConnection(tcp_server, &newNode->tcpConn);
			newNode->uid[0] = '\0'; newNode->rid=0; newNode->vc=0; newNode->tid=0;	// set as clean
			listInsert(userList, newNode);	// add to list pf tcp connections
			addSocket(&newNode->tcpConn, &fds, &fds_size);	// add to select fd set
			_VERBOSE("New user connection accepted.\nIP: %s\t Port: %d", tcpConnIp(&newNode->tcpConn), tcpConnPort(&newNode->tcpConn));
		}

		
		// Handle all tcp cliente connections
		iter = listIteratorCreate(userList);
		while (!listIteratorEmpty(&iter)){
			ListNode_t node = (ListNode_t) iter;
			userNode_t *nodeData = listIteratorNext(&iter);
			TCPConnection_t *conn = &nodeData->tcpConn;
			if (FD_ISSET(conn->fd, &ready_fds)){
				if(handleTCP(nodeData, msgBuf) == FALSE){
					LOG("connection closed");
					// connection closed
					removeSocket(conn, &fds, &fds_size);
					listRemove(userList, node, cleanListNodeUser);
				}
			}
		}

	}

	// after signal event, termiate
	exitAS(exitCode);
}



int main(int argc, char *argv[]) {
	initSignal(&terminateAS, &abortAS);	// sets the termination signals
	srand(time(NULL));					// set seed for random number
	/* AS makes available two server applications? Does it mean 2 process? */
	/* Default AS port. */        
	connectionInfo_t connectionInfo = {"58053\0"};
	parseArgs(argc, argv, &connectionInfo);
	dir = initDir(argv[0], DIR_NAME, mainDir_path);
	
	pdList = listCreate();
	userList = listCreate();
	
	// mount UDP server socket and pd logs
	udpServer = udpCreateServer(NULL, connectionInfo.asport);

	// mount TCP server socket and user logs
	tcpServer = tcpCreateServer(NULL, connectionInfo.asport, SOMAXCONN);
	
	waitMainEvent(tcpServer, udpServer, msgBuffer);

	return 0; // Never used
}