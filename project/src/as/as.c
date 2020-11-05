#include "as_aux.h"
#include "../common.h"
#include "../udp.h"
#include "../tcp.h"
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
DIR *dir;
char dir_path[PATH_MAX];
TCPConnection_t *userConnection;
UDPConnection_t *udpServer;
char msgBuffer[2*BUFFER_SIZE];	// prevent overflows, giving space to concatenate msgs
char verbosity = FALSE;

#define CHECK_VERBOSITY { return verbosity }


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


/* Handle UDP Responses (Incoming Messages) */
bool_t handleUDP(UDPConnection_t *udpConnec, char *msgBuf) {
	int n;
	char opcode[BUFFER_SIZE];
	
	n = udpReceiveMessage(udpConnec, msgBuf, BUFFER_SIZE);
	// TODO setClean();

	sscanf(msgBuf, "%s", opcode);

	// Registration Request
	if (!strcmp(opcode, REQ_REG))
		req_registerPD(udpConnec, msgBuf, dir_path);
	// Unregistration Request
	else if (!strcmp(opcode, REQ_UNR))
		;// TODO unregisterUser(respEnd);                
	// Validation Code received "VLC"
	else if (!strcmp(opcode, RESP_VLC))
		;// TODO validationCode_Response();
	else if (!strcmp(opcode, SERVER_ERR)) {
		WARN("Invalid request! Operation ignored.");
		return FALSE;
	}
	else{
		_WARN("Invalid opcode on the server response! Sending error. Got: %s", opcode);
		// return req_serverError(fd);
	}
}


void waitMainEvent(TCPConnection_t *userConnection, UDPConnection_t *udpConnec, char *msgBuf) {
	fd_set fds, ready_fds;
	struct timeval tv, tmp_tv;
	int selectRet, fds_size;
	int nTry = 0;
	int waitingReply = FALSE;
	/* SELECT */
	FD_ZERO(&fds);
	FD_SET(userConnection->fd, &fds);
	FD_SET(udpConnec->fd, &fds);
	fds_size = (userConnection->fd > udpConnec->fd) ? userConnection->fd+1 : udpConnec->fd+1;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;

	putStr(STR_INPUT, TRUE);		// string before the user input
	
	while (TRUE) {
		// because select is destructive
		ready_fds = fds;
		tmp_tv = tv;

		selectRet = select(fds_size, &ready_fds, NULL, NULL, &tmp_tv);

		if (selectRet == -1)
			FATAL("Failed System Call Select");
		if (FD_ISSET(udpConnec->fd , &ready_fds)){
			// handle PD interaction
			waitingReply = handleUDP(udpConnec, msgBuf);
		}
		if (FD_ISSET(userConnection->fd, &ready_fds)){
			// handle User new connection
		}
		if (selectRet == 0 && waitingReply) {// timeout expired
			// act as previous message didn't reach the target
			// try to resend NTRIES_NORESP times
			if (nTry < NREQUEST_TRIES)
				;//handle no reponse to prev msg
			else{
				nTry = 0;
				WARN("No response received from sent message.\nCommunication error.");
			}
		}       
		
	}
}




void exitAS() {
	udpDestroySocket(udpServer);
	tcpDestroySocket(userConnection);
	closedir(dir);
	exit(EXIT_SUCCESS);
}

void listDir(DIR* dir){
	struct dirent *ent;
		while ((ent = readdir(dir)) != NULL) {
		printf("%s\n", ent->d_name);
	}
}

int main(int argc, char *argv[]) {
	/* AS makes available two server applications? Does it mean 2 process? */
	/* Default AS port. */        
	connectionInfo_t connectionInfo = {"58053\0"};
	parseArgs(argc, argv, &connectionInfo);
	dir = initDir(argv[0], DIR_NAME, dir_path);
	// mount UDP server socket
	udpServer = udpCreateServer(NULL, connectionInfo.asport);
	// mount TCP server socket
	userConnection = tcpCreateServer(NULL, connectionInfo.asport, SOMAXCONN);

	waitMainEvent(userConnection, udpServer, msgBuffer);

	return 0; // Never used
}