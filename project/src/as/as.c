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
int exitCode = -1;
DIR *dir;
char dir_path[PATH_MAX];
TCPConnection_t *tcpServer;
UDPConnection_t *udpServer;
List_t tcpList;
char msgBuffer[2*BUFFER_SIZE];	// prevent overflows, giving space to concatenate msgs
char verbosity = FALSE;

// /* ========================== */
// /* Linked list Implementation */
// /* ========================== */
// void pushHead(int key, struct addrinfo data) {
// 	//create a new node
// 	udpNode_t *new = (udpNode_t*) malloc(sizeof(udpNode_t));

// 	// fill in the values
// 	new->id = key;
// 	new->addr = data;
// 	new->next = listHead;
	
// 	// updates head	
// 	listHead = new;
// }

// udpNode_t* delete(int key) {
// 	udpNode_t* current = listHead;
// 	udpNode_t* previous = NULL;
		
// 	// If empty
// 	if(listHead == NULL) return NULL;

// 	while(current->id != key) {
// 		//if enf of list
// 		if(current->next == NULL) return NULL;
// 		else {
// 			// move forward
// 			previous = current;
// 			current = current->next;
// 		}
// 	}

// 	// if match is list head
// 	if(current == listHead) 
// 		listHead = listHead->next;
// 	else 
// 		previous->next = current->next; //update
		
// 	return current;	
// }

// udpNode_t* find(int key) {
// 	// Start search from head
// 	udpNode_t* current = listHead;

// 	// If empty
// 	if(listHead == NULL) return NULL;

// 	// loop through queue until key match
// 	while(current->id != key) {
// 		if(current->next == NULL) return NULL;
// 		current = current->next;
// 	}      
		
// 	return current;
// }


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



void cleanListNodeTCP(void* nodeData) {
	asNodeTCP_t *nodeDataAS =  (asNodeTCP_t*) nodeData;
	tcpCloseConnection_noAlloc(nodeDataAS->tcpConn);
}

void exitAS(int flag) {
	udpDestroySocket(udpServer);
	tcpDestroySocket(tcpServer);
	listDestroy(tcpList, cleanListNodeTCP);
	closedir(dir);
	exit(flag);
}

/* Handle UDP Responses (Incoming Messages) */
bool_t handleUDP(UDPConnection_t *udpConnec, char *msgBuf) {
	int n;
	char opcode[BUFFER_SIZE];
	UDPConnection_t recvConnoc;
	n = udpReceiveMessage(udpConnec, &recvConnoc, msgBuf, BUFFER_SIZE);
	// TODO setClean();

	sscanf(msgBuf, "%s", opcode);

	// Registration Request
	if (!strcmp(opcode, REQ_REG)){
		req_registerPD(udpConnec, &recvConnoc, msgBuf+strlen(REQ_REG), dir_path);
		return FALSE;	// not waiting replay
	}// Unregistration Request
	else if (!strcmp(opcode, REQ_UNR)){
		req_unregisterPD(udpConnec, &recvConnoc, msgBuf+strlen(REQ_UNR), dir_path);                
		return FALSE;	// not waiting replay
	}
	// Validation Code received "RVC"
	else if (!strcmp(opcode, RESP_VLC))
		return FALSE;// TODO ? validationCode_Response();
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

bool_t handleTCP(asNodeTCP_t *tcpNode, char *msgBuf) {
	char opcode[BUFFER_SIZE];

	if (tcpReceiveMessage(&tcpNode->tcpConn, msgBuf, BUFFER_SIZE) == -1){
		unregisterUser(tcpNode, dir_path);
		return TRUE;	
	}

	sscanf(msgBuf, "%s", opcode);

	// Login Request
	if (!strcmp(opcode, REQ_LOG))
		req_loginUser(tcpNode, msgBuf+strlen(REQ_LOG), dir_path);
	// File manipulation Request
	else if (!strcmp(opcode, REQ_REQ))
		;// req_fileOP()
	// Authorize Op
	else if (!strcmp(opcode, REQ_AUT))
		;// req_Auth()
	// Error
	else if (!strcmp(opcode, SERVER_ERR)) {
		WARN("Invalid request! Operation ignored.");
		return TRUE;
	}
	else{
		_WARN("Invalid opcode on the server response! Sending error. Got: %s", opcode);
		req_serverErrorTCP(&tcpNode->tcpConn, msgBuf);
		return TRUE;
	}
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
	int waitingReply = FALSE;
	/* SELECT */
	FD_ZERO(&fds);
	FD_SET(tcp_server->fd, &fds);
	FD_SET(udp_server->fd, &fds);
	fds_size = (tcp_server->fd > udp_server->fd) ? tcp_server->fd+1 : udp_server->fd+1;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;
	
	while (exitCode != EXIT_FAILURE && exitCode != EXIT_SUCCESS) {
		// because select is destructive
		ready_fds = fds;
		tmp_tv = tv;

		selectRet = select(fds_size, &ready_fds, NULL, NULL, &tmp_tv);

		if (selectRet  == -1){
			if (errno == EINTR) break;	// return from signal
			_FATAL("Unable to start the select() to monitor the descriptors!\n\t - Error code: %d", errno);
		}

		// handle PD interaction
		if (FD_ISSET(udp_server->fd , &ready_fds)){
			waitingReply = handleUDP(udp_server, msgBuf);
		}

		// handle User new connection
		if (FD_ISSET(tcp_server->fd, &ready_fds)){
			asNodeTCP_t *newNode = (asNodeTCP_t*) malloc(sizeof(asNodeTCP_t));
			tcpAcceptConnection(tcp_server, &newNode->tcpConn);
			// add to list pf tcp connections
			listInsert(tcpList, newNode);	
			// add to select fd set
			addSocket(&newNode->tcpConn, &fds, &fds_size);
		}

		// timeout expired
		if (selectRet == 0 && waitingReply) {
			// act as previous message didn't reach the target
			// try to resend NTRIES_NORESP times
			if (nTry < NREQUEST_TRIES)
				;//handle no reponse to prev msg
			else{
				nTry = 0;
				WARN("No response received from sent message.\nCommunication error.");
			}
		}       
		
		if (selectRet != 0){	// not time out
			// Handle all tcp cliente connections
			ListIterator_t iter = listIteratorCreate(tcpList);
			while (!listIteratorEmpty(&iter)){
				puts("inloop");
				ListNode_t node = (ListNode_t) iter;
				asNodeTCP_t *nodeData = listIteratorNext(&iter);
				TCPConnection_t *conn = &nodeData->tcpConn;
				if (FD_ISSET(conn->fd, &ready_fds)){
					if(handleTCP(nodeData, msgBuf) == FALSE){
						// connection closed
						removeSocket(conn, &fds, &fds_size);
						listRemove(tcpList, node, cleanListNodeTCP);
					}
				}
			}
		}

	}

	// after signal event, termiate
	exitAS(exitCode);
}



// void listDir(DIR* dir){
// 	struct dirent *ent;
//     	while ((ent = readdir(dir)) != NULL) {
// 		printf("%s\n", ent->d_name);
// 	}
// }

int main(int argc, char *argv[]) {
	/* AS makes available two server applications? Does it mean 2 process? */
	/* Default AS port. */        
	connectionInfo_t connectionInfo = {"58053\0"};
	parseArgs(argc, argv, &connectionInfo);
	dir = initDir(argv[0], DIR_NAME, dir_path);
	// mount UDP server socket
	udpServer = udpCreateServer(NULL, connectionInfo.asport);
	// mount TCP server socket
	tcpServer = tcpCreateServer(NULL, connectionInfo.asport, SOMAXCONN);
	tcpList = listCreate();
	waitMainEvent(tcpServer, udpServer, msgBuffer);

	return 0; // Never used
}