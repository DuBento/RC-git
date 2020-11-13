#include "../common.h"
#include "user_aux.h"

TCPConnection_t *asConnection = NULL;
TCPConnection_t *fsConnection = NULL; //it's quite probable this one can get out of here and just be local	

int rid = RID_INVALID;
int tid = TID_INVALID;

/* File name being handled in retreive requests. */
char *filename = NULL;



connectionInfo_t connectionInfo = {"\0", "58053\0", "\0", "59053\0"};

userInfo_t userInfo = { 0 };


/*! \brief Cleans the program on termination
 *
 *	Frees all the memory alocated by the program and cleans terminates all the 
 *	required modules.
 */
void cleanUser() {
//	fcntl(0, F_SETFL, flags);

	if (asConnection != NULL)	tcpDestroySocket(asConnection);
	if (fsConnection != NULL)	tcpDestroySocket(fsConnection);

	if (filename != NULL)		free(filename);

	if (userInfo.uid != NULL) {		free(userInfo.uid);		}
	if (userInfo.pass != NULL) {		free(userInfo.pass);		}
}


/*! \brief Terminates the program on sucess.
 *
 *	Termination handle called by the SIGINT and SIGTERM signals.
 */
void terminateUser() {
	cleanUser();
	printf(MSG_SHUTDOWN" "MSG_THANKS"\n");
	exit(EXIT_SUCCESS);
}


/*! \brief Terminates the program on fatal errors.
 *
 *	Termination handle called by the SIGABRT, SIGFPE, SIGILL and SIGSEGV signals
 */
void abortUser() {
	cleanUser();
	printf(MSG_SHUTDOWN"\n");
	exit(EXIT_FAILURE);
}



void parseArgs(int argc, char *argv[]) {
		// check the number of arguments       
	if (argc < 1 || argc > 9 || argc % 2 != 1) {
		_FATAL("Invalid number of arguments!\n\t - [Usage]: %s [-n ASIP] "
		"[-p ASport] [-m FSIP] [-q FSport]\n", argv[0]);
	}

	for (int i = 1; i < argc; i++) {
		int ipPortSwitch = 0;
		if (!strcmp(ARG_ASIP, argv[i]) && (ipPortSwitch = ARG_IP) && isIPValid(argv[i + 1])) 
			strncpy(connectionInfo.asip, argv[++i], IP_SIZE);
		else if (!strcmp(ARG_ASPORT, argv[i]) && (ipPortSwitch = ARG_PORT) && isPortValid(argv[i + 1]))
			strncpy(connectionInfo.asport, argv[++i], PORT_SIZE);
		else if (!strcmp(ARG_FSIP, argv[i]) && (ipPortSwitch = ARG_IP) && isIPValid(argv[i + 1]))
			strncpy(connectionInfo.fsip, argv[++i], IP_SIZE); 
		else if (!strcmp(ARG_FSPORT, argv[i]) && (ipPortSwitch = ARG_PORT) && isPortValid(argv[i + 1]))
			strncpy(connectionInfo.fsport, argv[++i], PORT_SIZE);
		else {
			if (ipPortSwitch == ARG_IP) {
				_FATAL("Invalid " ARG_STR_IP " '%s'!""\n\t - [Usage]: "
				ARG_USAGE_IP " (x -> digit)", argv[i + 1])
			} else if (ipPortSwitch == ARG_PORT) {
				_FATAL("Invalid " ARG_STR_PORT " '%s'!""\n\t - [Usage]: "
				ARG_USAGE_PORT " (x -> digit)", argv[i + 1]) 
			} else
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
	char buffer[BUFFER_SIZE] = {0};
for(int i = 0; i < BUFFER_SIZE; i++) {
	buffer[i] = '\0';
}
	
	if (!getUserInput(buffer, BUFFER_SIZE))
		return FALSE;		// command ignored because the buffer overflowed

	char cmd[BUFFER_SIZE] = { 0 }, input1[BUFFER_SIZE] = { 0 }, 
	input2[BUFFER_SIZE] = { 0 };

//_LOG("buffer %s\t%d\t\tbuffer0 %x", buffer, cmd[0], buffer[0]);
	sscanf(buffer, "%s %s %s\n", cmd, input1, input2);
//_LOG("depois do scanf %d", cmd[0]);
	// login command: login UID pass
	if (!strcmp(cmd, CMD_LOGIN) && input1[0] != '\0' && input2[0] != '\0') {
		return req_login(asConnection, &userInfo, input1, input2);
	}
	// req command: req Fop [Fname]
	else if (!strcmp(cmd, CMD_REQ) && input1[0] != '\0') {
		rid = req_request(asConnection, &userInfo, input1, input2);
	}

	// val command: val VC
	else if (!strcmp(cmd, CMD_VAL) && input1[0] != '\0')
		return req_val(asConnection, &userInfo, input1, rid);

	// list command: list or l
	else if ((!strcmp(cmd, CMD_LIST) || !strcmp(cmd, CMD_LIST_S)) 
	&& input1[0] == '\0')
		userInfo.fsConnected = req_list(&fsConnection, &userInfo, tid);

	// retrieve command: retrieve filename or r filename
	else if ((!strcmp(cmd, CMD_RETRIEVE) || !strcmp(cmd, CMD_RETRIEVE_S)) 
	&& input1[0] != '\0' && input2[0] == '\0')
		userInfo.fsConnected = req_retrieve(&fsConnection, &userInfo, tid, input1, &filename);

	// upload command: upload filename or u filename
	else if ((!strcmp(cmd, CMD_UPLOAD) || !strcmp(cmd, CMD_UPLOAD_S)) 
	&& input1[0] != '\0' && input2[0] == '\0')
		userInfo.fsConnected = req_upload(&fsConnection, &userInfo, tid, input1);
			
	// delete command: delete filename or d filename
	else if ((!strcmp(cmd, CMD_DELETE) || !strcmp(cmd, CMD_DELETE_S)) 
	&& input1[0] != '\0' && input2[0] == '\0')
		userInfo.fsConnected = req_delete(&fsConnection, &userInfo, tid, input1);

	//remove command: remove or x 
	else if ((!strcmp(cmd, CMD_REMOVE) || !strcmp((cmd), CMD_REMOVE_S)) 
	&& input1[0] == '\0')
		userInfo.fsConnected = req_remove(&fsConnection, &userInfo, tid);

	// exit command: exit
	else if (!strcmp(cmd, CMD_EXIT) && input1[0] == '\0')
		terminateUser();
			 
	else {
//_LOG("cmd %s CMD_EXIT %s condition %d", cmd, CMD_EXIT, strcmp(cmd, CMD_EXIT) );
//int lencmd = strlen(cmd);
//int lenexit = strlen(CMD_EXIT);
//_LOG("len 1 %d, len 2 %d", lencmd, lenexit);
//for (int i = 0; i < lencmd; i++) {
//	_LOG("1st %c", cmd[i]);
//}
		WARN(MSG_ERR_INV_CMD" "MSG_OP_IGN"\n");
		return FALSE;
	}
	return FALSE;
}


bool_t handleASServer() {
	/* User always receives 1 arg from AS*/
	char buffer[BUFFER_SIZE] = {0}, opcode[BUFFER_SIZE] = { 0 }, 
	arg[BUFFER_SIZE] = {0};
	int size;
	
	size = tcpReceiveMessage(asConnection, buffer,BUFFER_SIZE);

	if (size == TCP_FLD_RCV) {
		asConnection = tcpDestroySocket(asConnection);
		return FALSE;
	}

	sscanf(buffer, "%s %s", opcode, arg);

	// Login response "RLO"
	if (!strcmp(opcode, RESP_LOG))
		userInfo.loggedIn = resp_login(&userInfo, arg);

	// Request code response "RRQ"	
	else if (!strcmp(opcode, RESP_REQ))
		return resp_request(arg);

	// Authentication response "RAU TID"
	else if (!strcmp(opcode, RESP_AUT))
		tid = resp_val(arg);

	else if (!strcmp(opcode, SERVER_ERR) && arg[0] == '\0') {
		printf(MSG_ERR_INV_REQ"\n.");
//		return FALSE;
	} else {
		printf(MSG_ERR_COM MSG_AS ".\n"MSG_SORRY"\n");
	}
	return TRUE;
}


bool_t handleFSServer() {
	char buffer[BUFSIZ] = {0}, opcode[BUFFER_SIZE] = {0}, *arg;
	int size;
	
	/* Each user can have a maximum of 15 files stored in the FS server. */
	/* All file Fsize fields can have at most 10 digits. */
	/* the filename Fname, limited to a total of 24 alphanumerical characters */
LOG("about to receive fs message");

	size = tcpReceiveMessage(fsConnection, buffer, BUFSIZ);

	if (size == TCP_FLD_RCV) {
		fsConnection = tcpDestroySocket(fsConnection);
		return FALSE;
	}
LOG("Received fs message");
//	buffer[strlen(buffer)-1] = '\0';
	_LOG("Le fs buffer %s", buffer);
_LOG("le le size %d", size);

	arg = buffer + PROTOCOL_MSSG_OFFSET;
	sscanf(buffer, "%s", opcode);
_LOG("le arg %s", arg);
	// List response RLS N[ Fname Fsize]*
	if (!strcmp(opcode, RESP_LST))
		userInfo.fsConnected = !resp_list(&fsConnection, arg);

	// Retrieve code response "RRT status [Fsize data]"	
	else if (!strcmp(opcode, RESP_RTV)) {
		
		userInfo.fsConnected = !resp_retrieve(&fsConnection, arg, &filename, size);

	}
	// Upload response " RUP status"
	else if (!strcmp(opcode, RESP_UPL))
		userInfo.fsConnected = !resp_upload(&fsConnection, arg);

	//	Delete response RDL status
	else if (!strcmp(opcode, RESP_DEL))
		userInfo.fsConnected = !resp_delete(&fsConnection, arg);

	//	Remove response RRM status
	else if (!strcmp(opcode, RESP_REM)) {
		userInfo.fsConnected = !resp_remove(&fsConnection, &userInfo, arg);
		terminateUser();
	} else if (!strcmp(opcode, SERVER_ERR) && arg[0] == '\0') {
		WARN(MSG_ERR_INV_REQ"\n");
	}
	return TRUE;
}


void runUser() {
	// select
	int fdsSize;
	fd_set fds;
	
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);		// user input 
	FD_SET(asConnection->fd, &fds);		// incoming messages from AS
	
	fdsSize = asConnection->fd + 1;
	
	// timeouts
	struct timeval tv;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;

	// Servers requests
	bool_t waitingReply = FALSE;
	int nRequestTries = 0;
	
	//putStr(STR_INPUT, TRUE);		// string before the user input
	while (TRUE) {
		fd_set fdsTemp = fds;		// select is destructive
		struct timeval tvTemp = tv;	// select is destructive
		fdsSize = asConnection->fd + 1;

		if (userInfo.fsConnected) {
			FD_SET(fsConnection->fd, &fdsTemp);
			fdsSize = fsConnection->fd + 1;	// is there a way not to do this all the time?
		}	

	//	fcntl(0, F_SETFL, flags | O_NONBLOCK);
	//	while (getchar() != EOF);
	//	fcntl(0, F_SETFL, flags);
		

		int selRetv = select(fdsSize, &fdsTemp, NULL, NULL, &tvTemp);
		if (selRetv  == -1) {
			cleanUser();
			_FATAL("Unable to start the select() to monitor the descriptors!\n\t - Error code: %d", errno);
		}
		// handle AS server responses
		if (FD_ISSET(asConnection->fd, &fdsTemp)) {
			//LOG("Yey as contacted us!");
			//putStr(STR_CLEAN, FALSE);		// clear the previous CHAR_INPUT
			//putStr(STR_RESPONSE, TRUE);		// string before the server output
			if (!handleASServer()) {
				printf(MSG_ERR_COM MSG_AS".\n"MSG_SORRY"\n");	
				return; 
			}	
			//putStr(STR_INPUT, TRUE);		// string before the user input
			waitingReply = FALSE;
		}

		if (userInfo.fsConnected && FD_ISSET(fsConnection->fd, &fdsTemp)) {
		// handle FS server responses
			//putStr(STR_CLEAN, FALSE);		// clear the previous CHAR_INPUT
			//putStr(STR_RESPONSE, TRUE);		// string before the server output
			//LOG("Yey fs contacted us!");
			if (!handleFSServer()) {
				printf(MSG_ERR_COM MSG_FS".\n"MSG_SORRY"\n");	
				return; 
			}
			//putStr(STR_INPUT, TRUE);		// string before the user input
			waitingReply = FALSE;
		}

		// handle stdin
		if (FD_ISSET(STDIN_FILENO, &fdsTemp)) {
			//putStr(STR_INPUT, TRUE);
			waitingReply = handleUser();
			//putStr(STR_INPUT, TRUE);		// string before the user input
		}
		
		if (selRetv == 0 && waitingReply) {
			if (nRequestTries == NREQUEST_TRIES) {
				// Exceeded max resends
				WARN("The server is "MSG_NOT_RESP"\n"MSG_OP_IGN "\n");
				waitingReply = FALSE;

			} else if (userInfo.asConnected) {
				WARN(MSG_AS " not responding. "
				MSG_RECONTACT"\n");
				waitingReply = req_resendLastMessage(asConnection);	// todo to change to fs also
				++nRequestTries;

			} else if (userInfo.fsConnected) {
				WARN(MSG_FS " not responding. "
				MSG_RECONTACT"\n");
				waitingReply = req_resendLastMessage(fsConnection);	// todo to change to fs also
				++nRequestTries;
			}
		}
	}
}


bool_t initUser() {
	/* Establish TCP connection with AS. */
	asConnection = tcpCreateClient(
		(connectionInfo.asip[0] == '\0' ? NULL : connectionInfo.asip), connectionInfo.asport);
	userInfo.asConnected = tcpConnect(asConnection);

	/* Initialise user info structure. */
	userInfo.fsConnected = FALSE;
	return TRUE;
}


int main(int argc, char *argv[]) { 
	
	srand(time(NULL));

	// Being sure user input is clean.
//	flags = fcntl(0, F_GETFL, 0);
	
	initSignal(&terminateUser, &abortUser);
	parseArgs(argc, argv);	

	initUser();
	runUser();

	terminateUser();
	return 0;
}