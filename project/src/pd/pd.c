#include <sys/select.h>

#include "common.h"
#include "udp.h"
#include "protocol.h"



static userInfo_t userInfo = {0};
static connectionInfo_t connectionInfo = {"", "57053\0", "193.136.138.142\0", "58011\0"};
// static connectionInfo_t connectionInfo = {"", "57053\0", "127.0.0.1\0", "58053\0"};
static int asSockfd = -1;




/*! \brief Cleans the program on termination
 *
 *	Frees all the memory alocated by the program and cleans terminates all the 
 *	required modules.
 */
void cleanPD() {
	if (asSockfd != -1)     udpDestroySocket(asSockfd);
	free(userInfo.uid);
	free(userInfo.pass);
}

/*! \brief Terminates the program on sucess.
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



#define ARG_IP          1
#define ARG_PORT 	2
#define ARG_STR_IP	"IP address"
#define ARG_STR_PORT	"Port number"
#define ARG_USAGE_IP	"xxx.xxx.xxx.xxx"
#define ARG_USAGE_PORT	"xxxxx"

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
			else
				_FATAL("Invalid " ARG_STR_PORT " '%s'!""\n\t - [Usage]: "
				ARG_USAGE_PORT " (x -> digit)", argv[i + 1])
		}			
	}

	/* logs the server information (on debug mod only) */
	_LOG("serverInfo settings:\nPDIP\t: %s\nPDport\t: %s\nASIP\t: %s\nASport\t: %s\n", 
	connectionInfo.pdip, connectionInfo.pdport, connectionInfo.asip, connectionInfo.asport);
}



/* User commands */
#define CMD_REG		"reg"		// register command
#define CMD_EXIT	"exit"		// exit command
#define CMD_REG_L	3		// length of the register command
#define CMD_EXIT_L	4		// length of the exit command


/*! \brief Handles the user input during the runtime.
 *
 *  Verifies which command was introduced by the user, validates its arguments and
 *  calls the apropriate protocol function.
 * 
 * \return TRUE if the message was well sent on to the server, FALSE otherwise.
 */
bool_t handleUser() {
	char buffer[BUFSIZ];
	if (!getUserInput(buffer, BUFSIZ))
		return FALSE;		// command ignored because the buffer overflowed

	// register command
	if (!strncmp(buffer, CMD_REG, CMD_REG_L) && buffer[CMD_REG_L] == CHAR_SEP_MSG) {
		// uid argument 
		char *uidArg = &buffer[CMD_REG_L];
		*(uidArg++)  = '\0';

		// pass argument
		char *passArg = strchr(uidArg, CHAR_SEP_MSG);
		if (passArg == NULL || passArg == uidArg) { 
			WARN("Invalid format for the 'reg' command!"); 
			return FALSE; 
		}
		*(passArg++) = '\0';

		// end of the arguments		
		char *endArg = strchr(passArg, CHAR_END_MSG);
		if (endArg == NULL || endArg == passArg || endArg[1] != '\0' || strchr(passArg, CHAR_SEP_MSG) != NULL) { 
			WARN("Invalid command! Operation ignored."); 
			return FALSE; 
		}		
		*(endArg++)  = '\0';

		// store the uid and pass and call the protocol function
		userInfo.uid  = (char*)malloc((passArg - uidArg) * sizeof(char));
		userInfo.pass = (char*)malloc((endArg - passArg) * sizeof(char));
		strcpy(userInfo.uid, uidArg);
		strcpy(userInfo.pass, passArg);
		return req_registerUser(asSockfd, &connectionInfo, &userInfo);
	}

	// exit command
	if (!strncmp(buffer, CMD_EXIT, CMD_EXIT_L) && buffer[CMD_EXIT_L] == '\n' && buffer[CMD_EXIT_L + 1] == '\0')
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
	char buffer[BUFSIZ];	
	int size = udpReceiveMessage(asSockfd, buffer, BUFSIZ);

	if (buffer[OPCODE_SIZE] != ' ' || buffer[size - 1] != '\n') {
		WARN("Invalid format on the server response! Operation ignored.");
		return FALSE;
	}
	buffer[size - 1] = '\0';

	// Registration response "RRG"
	if (!strncmp(buffer, RESP_REG, OPCODE_SIZE))
		return resp_registerUser(&buffer[OPCODE_SIZE + 1], &userInfo);

	// Validation code request "VLC"	
	if (!strncmp(buffer, REQ_VLC, OPCODE_SIZE))
		return req_valCode(asSockfd, &buffer[OPCODE_SIZE + 1], &userInfo);

	// Unegistration response "RUN"
	if (!strncmp(buffer, RESP_UNR, OPCODE_SIZE))
		return resp_unregisterUser(&buffer[OPCODE_SIZE + 1], &userInfo);
	
	WARN("Invalid opcode on the server response! Operation ignored.");
	return FALSE;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
void handleNoResponse(int sockfd, char* msgBuf) {
	udpSendMessage(sockfd, (const char*) msgBuf, BUFFER_SIZE);
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
				WARN("The server is not responding! Operation ignored");
			}
			else
				;//waitingReply = handleNoResponse(asSockfd);
		}       
	}
}



int main(int argc, char *argv[]) {
	initSignal(&terminatePD, &abortPD);	// sets the termination signals
	parseArgs(argc, argv);			// parses the execution arguments

	asSockfd = udpCreateClient(connectionInfo.asip, connectionInfo.asport);
	userInfo.connected = FALSE;

	runPD();

	/* first command should be reg (what if it's not?) DISPLAY ERR */

	/*  establish UDP client connection with AS server
			sends register message REG to AS
			receives RRG from AS
			while command is not exit
					waits for VLCs from AS and displays them
					sends RVC to AS server
			unregister user: send UNR to AS
			receives RUN response from AS
			shut down UDP client connection with AS server
	*/  
	/* 

	what if exit comes before reg?
	*/


	return 0; // Never used
}
