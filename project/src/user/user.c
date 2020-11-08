#include "../common.h"
#include "user_aux.h"
#include <string.h>

// tejo: IP=193.136.138.142). AS  (TCP/UDP) no porto 58011; FS TCP no porto 59000.
static connectionInfo_t connectionInfo = {TEJO_IP, TEJO_AS_PORT, TEJO_IP, TEJO_FS_PORT};
static userInfo_t userInfo = { 0 };
static TCPConnection_t *asConnection = NULL;
static TCPConnection_t *fsConnection = NULL;

static int rid;



/*! \brief Cleans the program on termination
 *
 *	Frees all the memory alocated by the program and cleans terminates all the 
 *	required modules.
 */
void cleanUser() {
	if (asConnection != NULL)   tcpDestroySocket(asConnection);
	if (fsConnection != NULL)	tcpDestroySocket(fsConnection);
  
  if (userInfo.uid != NULL)
		free(userInfo.uid);
	if (userInfo.pass != NULL)
		free(userInfo.pass);
}


/*! \brief Terminates the program on sucess.
 *
 *	Termination handle called by the SIGINT and SIGTERM signals.
 */
void terminateUser() {
	cleanUser();
	exit(EXIT_SUCCESS);
}


/*! \brief Terminates the program on fatal errors.
 *
 *	Termination handle called by the SIGABRT, SIGFPE, SIGILL and SIGSEGV signals
 */
void abortUser() {
	cleanUser();
	exit(EXIT_FAILURE);
}



void parseArgs(int argc, char *argv[]) {
		// check the number of arguments       
	if (argc < 1 || argc > 9 || argc % 2 != 1) {
		_FATAL("Invalid number of arguments!\n\t - [Usage]: %s [-n ASIP] "
		"[-p ASport] [-n FSIP] [-q FSport]\n", argv[0]);
	}

		for (int i = 1; i < argc; i++) {
				int ipPortSwitch = 0;
				if (!strcmp(ARG_ASIP, argv[i]) && (ipPortSwitch = ARG_IP) && isIPValid(argv[i + 1])) 
						strncpy(connectionInfo.asip, argv[++i], IP_SIZE);
				else if (!strcmp(ARG_ASPORT, argv[i]) && (ipPortSwitch = ARG_PORT) && isPortValid(argv[i + 1]))
						strncpy(connectionInfo.asip, argv[++i], PORT_SIZE);
				else if (!strcmp(ARG_FSIP, argv[i]) && (ipPortSwitch = ARG_IP) && isIPValid(argv[i + 1]))
					   strncpy(connectionInfo.asport, argv[++i], IP_SIZE); 
				else if (!strcmp(ARG_FSPORT, argv[i]) && (ipPortSwitch = ARG_PORT) && isPortValid(argv[i + 1]))
					   strncpy(connectionInfo.asport, argv[++i], PORT_SIZE);
				else {
			if (ipPortSwitch == ARG_IP)
				_FATAL("Invalid " ARG_STR_IP " '%s'!""\n\t - [Usage]: "
				ARG_USAGE_IP " (x -> digit)", argv[i + 1])
			else if (ipPortSwitch == ARG_PORT)
				_FATAL("Invalid " ARG_STR_PORT " '%s'!""\n\t - [Usage]: "
				ARG_USAGE_PORT " (x -> digit)", argv[i + 1])
			else
				FATAL("Invalid execution argument flag!\n\t - [Flags]: '-n', '-p', '-m', '-q'");
		}
		}

		/* logs the server information (on debug mod only) */
		_LOG("connectionInfo settings:\nASIP\t: %s\nASport\t: %s\nFSIP\t: %s\nFSport\t: %s\n", 
				connectionInfo.asip, connectionInfo.asport, connectionInfo.fsip, connectionInfo.fsport);
}



/*! \brief Handles the user input during the runtime.
 *
 *  Verifies which command was introduced by the user, validates its arguments and
 *  calls the apropriate protocol function.
 * 
 * \return TRUE if the message was well sent on to the server, FALSE otherwise.
 */
bool_t handleUser() {
	char buffer[BUFFER_SIZE];
	
	if (!getUserInput(buffer, BUFFER_SIZE))
		return FALSE;		// command ignored because the buffer overflowed

	char cmd[BUFFER_SIZE] = { 0 }, input1[BUFFER_SIZE] = { 0 }, input2[BUFFER_SIZE] = { 0 };
	sscanf(buffer, "%s %s %s", cmd, input1, input2);

	_LOG("[handleUser] cmd: %s\n input1: %s\n input2: %s", cmd, input1, input2);

	// login command: login UID pass
	if (!strcmp(cmd, CMD_LOGIN) && input1[0] != '\0' && input2[0] != '\0') {
		req_login(asConnection, &userInfo, input1, input2);
	}
	// req command: req Fop [Fname]
	else if (!strcmp(cmd, CMD_REQ) && input1[0] != '\0')
		return req_request(asConnection, &userInfo, input1, input2, &rid);

	// val command: val VC
	else if (!strcmp(cmd, CMD_VAL) && input1[0] != '\0')
		return req_val(asConnection, &userInfo, input1, &rid);

	// list command: list or l
	else if ((!strcmp(cmd, CMD_LIST) || !strcmp(cmd, CMD_LIST_S)) && input1[0] == '\0')
		;//return req_list();

	// retrieve command: retrieve filename or r filename
	else if ((!strcmp(cmd, CMD_RETRIEVE) || !strcmp(cmd, CMD_RETRIEVE_S)) && input1[0] != '\0' && input2[0] == '\0')
		;//return req_retrieve();

	// upload command: upload filename or u filename
	else if ((!strcmp(cmd, CMD_UPLOAD) || !strcmp(cmd, CMD_UPLOAD_S)) && input1[0] != '\0' && input2[0] == '\0')
		;//return req_upload();
			
	// delete command: delete filename or d filename
	else if ((!strcmp(cmd, CMD_DELETE) || !strcmp(cmd, CMD_DELETE_S)) && input1[0] != '\0' && input2[0] == '\0')
		;//return req_delete();

	//remove command: remove or x 
	else if ((!strcmp(cmd, CMD_REMOVE) || !strcmp((cmd), CMD_REMOVE_S)) && input1[0] == '\0')
		;//return req_remove();

	// exit command: exit
	else if (!strcmp(cmd, CMD_EXIT) && input1[0] == '\0')
		terminateUser();
			 
	else {
		WARN("Invalid command! Operation ignored.");
		return FALSE;
	}
	return FALSE;
}


bool_t handleASServer() {
	/* User always receives 1 arg from AS*/
	char buffer[BUFFER_SIZE] = {0}, opcode[BUFFER_SIZE] = { 0 }, arg[BUFFER_SIZE] = {0};
	int size;
	
	size = tcpReceiveMessage(asConnection, buffer,BUFFER_SIZE);
	sscanf(buffer, "%s %s", opcode, arg);
_LOG("AS contact: opcode %s, arg %s", opcode, arg);
	// Login response "RLO"
	if (!strcmp(opcode, RESP_LOG))
		return resp_login(arg);

	// Request code response "RRQ"	
	else if (!strcmp(opcode, RESP_REQ))
		return resp_request(arg);

	// Authentication response "RAU"
	else if (!strcmp(opcode, RESP_AUT))
		return resp_val(arg);

	else if (!strcmp(opcode, SERVER_ERR) && arg[0] == '\0') {
		WARN("Invalid request! Operation ignored.");
		return FALSE;
	}
	return TRUE;
}


bool_t handleFSServer() {
	char buffer[BUFFER_SIZE] = {0}, opcode[BUFFER_SIZE] = { 0 }, arg[BUFFER_SIZE] = {0};
	int size;
	
	/* Each user can have a maximum of 15 files stored in the FS server. */
	/* All file Fsize fields can have at most 10 digits. */
	/* the filename Fname, limited to a total of 24 alphanumerical characters */
	
	
	size = tcpReceiveMessage(fsConnection, buffer,BUFFER_SIZE);
	sscanf(buffer, "%s %s", opcode, arg);

	// List response RLS N[ Fname Fsize]*
	if (!strcmp(opcode, RESP_LOG))
		return resp_list(arg);

	// Retrieve code response "RRT status [Fsize data]"	
	else if (!strcmp(opcode, RESP_REQ))
		return resp_retrieve(arg);

	// Upload response " RUP status"
	else if (!strcmp(opcode, RESP_AUT))
		return resp_upload(fsConnection, arg);

	//	Delete response RDL status
	else if (!strcmp(opcode, RESP_AUT))
		return resp_delete(fsConnection, arg);

	//	Remove response RRM status
	else if (!strcmp(opcode, RESP_AUT))
		return resp_remove(fsConnection, arg);

	else if (!strcmp(opcode, SERVER_ERR) && arg[0] == '\0') {
		WARN("Invalid request! Operation ignored.");
		return FALSE;
	}
	return TRUE;
}


/*! \brief Main loop for the FS application.
 *
 *  Waits for an interaction from the user/server and then handles them.
 */
void runUser() {
	// select
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);		// only user input 
	FD_SET(asConnection->fd, &fds);			// incoming messages from AS
	/* if (userInfo.fsConnected)
	FD_SET(fsSockfd, &fds);
	*/

	// if userInfo.fsConnected: +2?
	int fdsSize = asConnection->fd + 1;
	
	// timeouts
	struct timeval tv;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;

	// server requests
	bool_t waitingReply = FALSE;
	int nRequestTries = 0;
	
	putStr(STR_INPUT, TRUE);		// string before the user input
	while (TRUE) {
		fd_set fdsTemp = fds;		// select is destructive
		struct timeval tvTemp = tv;	// select is destructive
		int selRetv = select(fdsSize, &fdsTemp, NULL, NULL, &tvTemp);
		if (selRetv  == -1)
			_FATAL("Unable to start the select() to monitor the descriptors!\n\t - Error code: %d", errno);

		// handle AS server responses
		if (FD_ISSET(asConnection->fd, &fdsTemp)) {
			//LOG("Yey as contacted us!");
			putStr(STR_CLEAN, FALSE);		// clear the previous CHAR_INPUT
			putStr(STR_RESPONSE, TRUE);		// string before the server output
			handleASServer();	
			putStr(STR_INPUT, TRUE);		// string before the user input
			waitingReply = FALSE;
		}

		// if (userInfo.fsConnected)
		// handle FS server responses
		/*if (FD_ISSET(fsSockfd, &fdsTemp)) {
			putStr(STR_CLEAN, FALSE);		// clear the previous CHAR_INPUT
			putStr(STR_RESPONSE, TRUE);		// string before the server output
			//handleFSServer();	
			putStr(STR_INPUT, TRUE);		// string before the user input
			waitingReply = FALSE;
		}*/

		// handle stdin
		if (FD_ISSET(STDIN_FILENO, &fdsTemp)) {
			waitingReply = handleUser();
			putStr(STR_INPUT, TRUE);		// string before the user input
		}
		
		if (selRetv == 0 && waitingReply) {
			if (nRequestTries == NREQUEST_TRIES) {
				// Exceeded max resends
				WARN("The server is not responding! Operation ignored");
				waitingReply = FALSE;
			}
			else
				waitingReply = req_resendLastMessage(asConnection);
		}
			
	}
}





int main(int argc, char *argv[]) { 
	srand(time(NULL));
		
	initSignal(&terminateUser, &abortUser);
	parseArgs(argc, argv);	

	/* Establish TCP connection with AS. */
	asConnection = tcpCreateClient(connectionInfo.asip, connectionInfo.asport);
	tcpConnect(asConnection);	
	runUser();

	terminateUser();
	return 0;
}