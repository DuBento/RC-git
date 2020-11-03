#include <sys/select.h>

#include "../common.h"
#include "../udp.h"
#include "pd_aux.h"



static userInfo_t userInfo = {0};
static connectionInfo_t connectionInfo = {"", "57053\0", "193.136.138.142\0", "58011\0"};
// static connectionInfo_t connectionInfo = {"", "57053\0", "127.0.0.1\0", "58053\0"};
static int asSockfd = -1;

// fd to the socket in which PD acts as an UDP server
static int pdSockfd = -1;

/* User commands */
#define CMD_REG		"reg"		// register command
#define CMD_EXIT	"exit"		// exit command


/*! \brief Cleans the program on termination
 *
 *	Frees all the memory alocated by the program and cleans terminates all the 
 *	required modules.
 */
void cleanPD() {
	if(userInfo.connected)  req_unregisterUser(asSockfd, &userInfo);
	if (asSockfd != -1)     udpDestroySocket(asSockfd);
	if (pdSockfd != -1)		udpDestroySocket(pdSockfd);
	free(userInfo.uid);
	free(userInfo.pass);
}


/*! \brief Terminates the program on success.
 *
 *	Termination handle called by the SIGINT and SIGTERM signals.
 */
void terminatePD() {
	cleanPD();
	exit(EXIT_SUCCESS);
}


/*! \brief Terminates the program on fatal errors.
 *
 *	Termination handle called by the SIGABRT, SIGFPE, SIGILL and SIGSEGV signals
 */
void abortPD() {
	cleanPD();
	exit(EXIT_FAILURE);
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

	char cmd[BUFFER_SIZE] = { 0 }, uid[BUFFER_SIZE] = { 0 }, pass[BUFFER_SIZE] = { 0 };
	sscanf(buffer, "%s %s %s", cmd, uid, pass);

	// register command
	if (!strcmp(cmd, CMD_REG) && uid[0] != '\0' && pass[0] != '\0')
		return req_registerUser(asSockfd, &connectionInfo, uid, pass, &userInfo);

	// exit command
	if (!strcmp(cmd, CMD_EXIT) && uid[0] == '\0')
		return req_unregisterUser(asSockfd, &userInfo);
		
	WARN("Invalid command! Operation ignored.");
	return FALSE;
}


/*! \brief Handles the server messages during the runtime.
 *
 *  Verifies which message was sent by the server and updates the program accordingly
 * 
 * \return TRUE if the message was well received from the server, FALSE otherwise.
 */
bool_t handleServer() {
	char buffer[BUFFER_SIZE], opcode[BUFFER_SIZE] = { 0 }, args[BUFFER_SIZE] = { 0 };	
	int size = udpReceiveMessage(asSockfd, buffer, BUFFER_SIZE);
	sscanf(buffer, "%s %s\n", opcode, args);

	// Registration response "RRG"
	if (!strcmp(opcode, RESP_REG))
		return resp_registerUser(args, &userInfo);

	// Validation code request "VLC"	
	else if (!strcmp(opcode, REQ_VLC))
		return req_valCode(asSockfd, args, &userInfo);

	// Unegistration response "RUN"
	else if (!strcmp(opcode, RESP_UNR))
		return resp_unregisterUser(args, &userInfo);

	else if (!strcmp(opcode, SERVER_ERR) && args[0] == '\0') {
		WARN("Invalid request! Operation ignored.");
		return FALSE;
	}
	
	else{
		_WARN("Invalid opcode on the server response! Sending error. Got: %s", opcode);
		return req_serverError(asSockfd);
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
	FD_SET(asSockfd, &fds);
	int fdsSize = asSockfd + 1;
	
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

		// handle server responses
		if (FD_ISSET(asSockfd, &fdsTemp)) {
			putStr(STR_CLEAN, FALSE);		// clear the previous CHAR_INPUT
			putStr(STR_RESPONSE, TRUE);		// string before the server output
			handleServer();	
			putStr(STR_INPUT, TRUE);		// string before the user input
			waitingReply = FALSE;
		}

		// handle stdin
		if (FD_ISSET(STDIN_FILENO, &fdsTemp)) {
			waitingReply = handleUser();
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
			else {
				waitingReply = req_resendLastMessage(asSockfd);
				nRequestTries++;
			}
		}
	}
}


int main(int argc, char *argv[]) {
	initSignal(&terminatePD, &abortPD);	// sets the termination signals
	parseArgs(argc, argv);			// parses the execution arguments

	asSockfd = udpCreateClient(connectionInfo.asip, connectionInfo.asport);
	userInfo.connected = FALSE;

	//pdSockfd = udpCreateServer(connectionInfo.pdip, connectionInfo.pdport);

	runPD();
	return 0;
}