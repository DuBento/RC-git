#include <sys/select.h>

#include "../common.h"
#include "../udp.h"
#include "pd_aux.h"


int exitCode = -1;
static userInfo_t userInfo = {0};
//static connectionInfo_t connectionInfo = {"", "57053\0", """, "58011\0"};
static connectionInfo_t connectionInfo = {"", "57053\0", "193.136.138.142\0", "58011\0"};

static UDPConnection_t *asConnection = NULL;
static UDPConnection_t *pdConnection = NULL;		// fd to the socket in which PD acts as an UDP server

/* User commands */
#define CMD_REG		"reg"		// register command
#define CMD_EXIT	"exit"		// exit command





bool_t handleClient(UDPConnection_t *udpConnec, fd_set *fds, int *fdsSize);

/*! \brief Cleans the program on termination
 *
 *	Frees all the memory alocated by the program and cleans terminates all the 
 *	required modules.
 */
void cleanPD() {
	if (asConnection == NULL)
			asConnection = udpCreateClient(connectionInfo.asip, connectionInfo.asport);
			
	if(userInfo.connected) {
		req_unregisterUser(asConnection, &userInfo);
		handleClient(asConnection, NULL, NULL);
	}

	if (asConnection != NULL)   udpDestroySocket(asConnection);
	if (pdConnection != NULL)	udpDestroySocket(pdConnection);
	if (userInfo.uid != NULL) 	free(userInfo.uid);
	if (userInfo.pass != NULL) 	free(userInfo.pass);
}


/*! \brief Set program to terminate on success.
 *
 *	Termination handle called by the SIGINT and SIGTERM signals.
 */
void terminatePD() {
	exitCode = EXIT_SUCCESS;
}


/*! \brief Set program to terminate on fatal errors.
 *
 *	Termination handle called by the SIGABRT, SIGFPE, SIGILL and SIGSEGV signals
 */
void abortPD() {
	exitCode = EXIT_FAILURE;
}



/*! \brief Parses the execution arguments.
 *
 * 	Validates the execution arguments and sets the connection settings.
 *
 * \param 	argc The number of execution arguments.
 * \param 	argv An array with the execution arguments.
 * \param 	info The instance that stores the connection settings.
 * \return	NULL.
 */
void parseArgs(int argc, char *argv[]) {
	// check the number of arguments
	if (argc < 2 || argc > 8 || argc % 2 != 0)
		_FATAL("Invalid number of arguments!\n\t - [Usage]: %s PDIP "
		"[-d PDport] [-n ASIP] [-p ASport]\n", argv[0]);

	if (!isIPValid(argv[1])) 
		_FATAL("Invalid " ARG_STR_IP " '%s'!""\n\t - [Usage]: " ARG_USAGE_IP " (x -> digit)", argv[1]);
		
	strncpy(connectionInfo.pdip, argv[1], IP_SIZE);

	for (int i = 2; i < argc; i++) {
		int ipPortSwitch = 0;
		if (!strcmp(ARG_PDPORT, argv[i]) && (ipPortSwitch = ARG_PORT) && isPortValid(argv[i + 1])) 
			strncpy(connectionInfo.pdport, argv[++i], PORT_SIZE);
		else if (!strcmp(ARG_ASIP, argv[i]) && (ipPortSwitch = ARG_IP) && isIPValid(argv[i + 1]))
			strncpy(connectionInfo.asip, argv[++i], IP_SIZE);
		else if (!strcmp(ARG_ASPORT, argv[i]) && (ipPortSwitch = ARG_PORT) && isPortValid(argv[i + 1]))
			strncpy(connectionInfo.asport, argv[++i], PORT_SIZE); 
		else {
			if (ipPortSwitch == ARG_IP)
				_FATAL("Invalid " ARG_STR_IP " '%s'!""\n\t - [Usage]: "
				ARG_USAGE_IP " (x -> digit)", argv[i + 1])
			else if (ipPortSwitch == ARG_PORT)
				_FATAL("Invalid " ARG_STR_PORT " '%s'!""\n\t - [Usage]: "
				ARG_USAGE_PORT " (x -> digit)", argv[i + 1])
			else
				FATAL("Invalid execution argument flag!\n\t - [Flags]: '-d','-n', '-p'");
		}			
	}

	/* logs the server information (on debug mod only) */
	_LOG("serverInfo settings:\nPDIP\t: %s\nPDport\t: %s\nASIP\t: %s\nASport\t: %s\n", 
	connectionInfo.pdip, connectionInfo.pdport, connectionInfo.asip, connectionInfo.asport);
}



void removeSocketClient(fd_set *fds, int *fdsSize) {
	if (fds == NULL || fdsSize == NULL)	return;
	int fd = asConnection->fd;
	if (*fdsSize == (fd + 1))	*fdsSize--;
	FD_CLR(fd, fds);
	udpDestroySocket(asConnection);
	asConnection = NULL;
}

void addSocket(UDPConnection_t *udpConnec, fd_set *fds, int *fdsSize) {
	if (*fdsSize < udpConnec->fd+1)	*fdsSize = udpConnec->fd+1;
	FD_SET(udpConnec->fd, fds);
}

/*! \brief Handles the user input during the runtime.
 *
 *  Verifies which command was introduced by the user, validates its arguments and
 *  calls the apropriate protocol function.
 * 
 * \return TRUE if the message was well sent on to the server, FALSE otherwise.
 */
bool_t handleUser(fd_set *fds, int *fdsSize) {
	char buffer[BUFFER_SIZE];
	if (!getUserInput(buffer, BUFFER_SIZE))
		return FALSE;		// command ignored because the buffer overflowed

	char cmd[BUFFER_SIZE] = { 0 }, uid[BUFFER_SIZE] = { 0 }, pass[BUFFER_SIZE] = { 0 };
	sscanf(buffer, "%s %s %s", cmd, uid, pass);

	// register command
	if (!strcmp(cmd, CMD_REG) && uid[0] != '\0' && pass[0] != '\0') {
		if (asConnection == NULL) {
			asConnection = udpCreateClient(connectionInfo.asip, connectionInfo.asport);
			addSocket(asConnection, fds, fdsSize);
		}
		return req_registerUser(asConnection, &connectionInfo, uid, pass, &userInfo);
	}

	// exit command
	if (!strcmp(cmd, CMD_EXIT) && uid[0] == '\0'){
		if (asConnection == NULL) {
			asConnection = udpCreateClient(connectionInfo.asip, connectionInfo.asport);
			addSocket(asConnection, fds, fdsSize);
		}
		return req_unregisterUser(asConnection, &userInfo);
	}
		
	WARN("Invalid command! Operation ignored.");
	return FALSE;
}


/*! \brief Handles the server messages during the runtime.
 *
 *  Verifies which message was sent by the server and updates the program accordingly.
 * 	This messages are the ones the server sends to the PD's UDP server.
 * 
 * \return TRUE if the message was well received from the server, FALSE otherwise.
 */
bool_t handleServer(UDPConnection_t *udpConnec) {
	char buffer[BUFFER_SIZE], opcode[BUFFER_SIZE];	
	UDPConnection_t receiver;
	int size = udpReceiveMessage(udpConnec, &receiver, buffer, BUFFER_SIZE);
	int validArgs = sscanf(buffer, "%s", opcode);

	// Validation code request "VLC"	
	if (validArgs == 1 && !strcmp(opcode, REQ_VLC))
		return resp_valCode(udpConnec, &receiver, buffer, &userInfo);

	else if (validArgs == 1 && !strcmp(opcode, SERVER_ERR)) {
		WARN("Invalid request! Operation ignored.");
		return FALSE;
	}
	
	else{
		_WARN("Invalid opcode on the server response! Sending error...\n\tGot\t: %s", opcode);
		return req_serverError(udpConnec);
	}
}

/*! \brief Handles the server messages during the runtime.
 *
 *  Verifies which message was sent by the server and updates the program accordingly
 * 
 * \return TRUE if the message was well received from the server, FALSE otherwise.
 */
bool_t handleClient(UDPConnection_t *udpConnec, fd_set *fds, int *fdsSize) {
	char buffer[BUFFER_SIZE], opcode[BUFFER_SIZE], args[BUFFER_SIZE];	
	int size = udpReceiveMessage(udpConnec, NULL, buffer, BUFFER_SIZE);
	int validArgs = sscanf(buffer, "%s %s\n", opcode, args);

	// Registration response "RRG"
	if (validArgs == 2 && !strcmp(opcode, RESP_REG) && fds != NULL && fdsSize != NULL) {
		if (resp_registerUser(args, &userInfo)) { // hable to register
			removeSocketClient(fds, fdsSize);	// removes client socket
			if (pdConnection == NULL) {
				pdConnection = udpCreateServer(connectionInfo.pdip, connectionInfo.pdport);
				addSocket(pdConnection, fds, fdsSize);
				_LOG("Server fd: %d, fdssize: %d", pdConnection->fd, *fdsSize);
			}
		}
	}

	// Unegistration response "RUN"
	else if (validArgs == 2 && !strcmp(opcode, RESP_UNR))
		return resp_unregisterUser(args, &userInfo);

	else if (validArgs == 1 && !strcmp(opcode, SERVER_ERR)) {
		WARN("Invalid request! Operation ignored.");
		return FALSE;
	}
	
	else{
		_WARN("Invalid opcode on the server response! Sending error. Got: %s", opcode);
		return req_serverError(udpConnec);
	}
}






/*! \brief Main loop for the PD application.
 *
 *  Waits for an interaction from the user/server and then handles them.
 */
void runPD() {
	// select
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);		// only user input 
	int fdsSize = STDIN_FILENO + 1;
	
	// timeouts
	struct timeval tv;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;

	// server requests
	bool_t waitingReply = FALSE;
	int nRequestTries = 0;
	
	putStr(STR_INPUT, TRUE);			// string before the user input
	while (exitCode != EXIT_FAILURE && exitCode != EXIT_SUCCESS) {
		fd_set fdsTemp = fds;			// select is destructive
		struct timeval tvTemp = tv;		// select is destructive
		int selRetv = select(fdsSize, &fdsTemp, NULL, NULL, &tvTemp);
		if (selRetv  == -1) {
			if (errno == EINTR) break;	// return from signal
			_FATAL("Unable to start the select() to monitor the descriptors!\n\t - Error code: %d", errno);
		}

		// handle server messages
		if (pdConnection != NULL && FD_ISSET(pdConnection->fd, &fdsTemp)) {
			putStr(STR_CLEAN, FALSE);		// clear the previous CHAR_INPUT
			putStr(STR_RESPONSE, TRUE);		// string before the server output
			handleServer(pdConnection);	
			putStr(STR_INPUT, TRUE);		// string before the user input
			waitingReply = FALSE;
		}

		// handle server responses
		if (asConnection != NULL && FD_ISSET(asConnection->fd, &fdsTemp)) {
			putStr(STR_CLEAN, FALSE);		// clear the previous CHAR_INPUT
			putStr(STR_RESPONSE, TRUE);		// string before the server output
			handleClient(asConnection, &fds, &fdsSize);	
			putStr(STR_INPUT, TRUE);		// string before the user input
			waitingReply = FALSE;
		}

		// handle stdin
		if (FD_ISSET(STDIN_FILENO, &fdsTemp) && !waitingReply) {
			waitingReply = handleUser(&fds, &fdsSize);
			putStr(STR_INPUT, TRUE);		// string before the user input
		}
		
		if (selRetv == 0 && waitingReply) {
			if (nRequestTries == NREQUEST_TRIES) {
				nRequestTries = 0;
				putStr(STR_CLEAN, FALSE);		// clear the previous CHAR_INPUT
				WARN("The server is not responding! Operation ignored");
				waitingReply = FALSE;
				putStr(STR_INPUT, TRUE);		// string before the user input
			}
			else if (asConnection != NULL){
				waitingReply = req_resendLastMessage(asConnection);
				nRequestTries++;
			}
		}
	}

	putStr(STR_CLEAN, FALSE);		// clear the previous CHAR_INPUT
	// terminate
	cleanPD();
	exit(exitCode);
}


int main(int argc, char *argv[]) {
	initSignal(&terminatePD, &abortPD);	// sets the termination signals
	parseArgs(argc, argv);				// parses the execution arguments

	userInfo.connected = FALSE;
	runPD();
	
	return 0;
}