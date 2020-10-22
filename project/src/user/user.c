#include "../common.h"
#include "user_aux.h"
#include <string.h>


static connectionInfo_t connectionInfo = {"", "58053\0", "193.136.138.142\0", "58011\0"};
static int asSockfd = -1;
static int fsSockfd = -1;



/*! \brief Cleans the program on termination
 *
 *	Frees all the memory alocated by the program and cleans terminates all the 
 *	required modules.
 */
void cleanFS() {
        // if(userInfo.connected)  req_unregisterUser(asSockfd, &userInfo);
	// close tcp conection
        if (asSockfd != -1)     tcpDestroySocket(asSockfd);
	if (fsSockfd != -1)	tcpDestroySocket(fsSockfd);
}

/*! \brief Terminates the program on sucess.
 *
 *	Termination handle called by the SIGINT and SIGTERM signals.
 */
void terminateFS() {
	cleanFS();
	exit(EXIT_SUCCESS);
}

/*! \brief Terminates the program on fatal errors.
 *
 *	Termination handle called by the SIGABRT, SIGFPE, SIGILL and SIGSEGV signals
 */
void abortFS() {
	cleanFS();
	exit(EXIT_FAILURE);
}



void parseArgs(int argc, char *argv[]) {
        // check the number of arguments       
	if (argc < 1 || argc > 9 || argc % 2 != 1) {
		_FATAL("Invalid number of arguments!\n\t - [Usage]: %s [-n ASIP] "
		"[-d ASport] [-n FSIP] [-p FSport]\n", argv[0]);
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
                info->asip, info->asport, info->fsip, info->fsport);
}



/* User commands */
#define CMD_LOGIN	"login"

#define CMD_REQ 	"req"

#define CMD_VAL         "val"

#define CMD_LIST        "list"
#define CMD_LIST_S      "l"

#define CMD_RETRIEVE    "retrieve"
#define CMD_RETRIEVE_S  "r"

#define CMD_UPLOAD      "upload"
#define CMD_UPLOAD_S    "u"

#define CMD_DELETE      "delete"
#define CMD_DELETE_S    "d"

#define CMD_REMOVE      "remove"
#define CMD_REMOVE_S    "x"

#define CMD_EXIT        "exit"

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
	if (!strcmp(cmd, CMD_LOGIN) && input1[0] != '\0' && input2[0] != '\0')
		return TRUE;

	// req command: req Fop [Fname]
	else if (!strcmp(cmd, CMD_REQ) && input1[0] != '\0')
		return TRUE;

        // val command: val VC
        else if (!strcmp(cmd, CMD_VAL) && input1[0] != '\0')
		return TRUE;

        // list command: list or l
        else if ((!strcmp(cmd, CMD_LIST) || !strcmp(cmd, CMD_LIST_S)) && input1[0] == '\0')
		return TRUE;

        // retrieve command: retrieve filename or r filename
        else if ((!strcmp(cmd, CMD_RETRIEVE) || !strcmp(cmd, CMD_RETRIEVE_S)) && input1[0] != '\0' && input2[0] == '\0')
		return TRUE;

        // upload command: upload filename or u filename
        else if ((!strcmp(cmd, CMD_UPLOAD) || !strcmp(cmd, CMD_UPLOAD_S)) && input1[0] != '\0' && input2[0] == '\0')
		return TRUE;
                
        // delete command: delete filename or d filename
        else if ((!strcmp(cmd, CMD_DELETE) || !strcmp(cmd, CMD_DELETE_S)) && input1[0] != '\0' && input2[0] == '\0')
		return TRUE;

        //remove command: remove or x 
        else if ((!strcmp(cmd, CMD_REMOVE) || !strcmp((cmd), CMD_REMOVE_S)) && input1[0] == '\0')
		return TRUE;

        // exit command: exit
        else if (!strcmp(cmd, CMD_EXIT) && input1[0] == '\0')
		return TRUE;
             
	WARN("Invalid command! Operation ignored.");
	return FALSE;
}



/*! \brief Main loop for the FS application.
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

		// handle AS server responses
		if (FD_ISSET(asSockfd, &fdsTemp)) {
			putStr(STR_CLEAN, FALSE);		// clear the previous CHAR_INPUT
			putStr(STR_RESPONSE, TRUE);		// string before the server output
			//handleASServer();	
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
				WARN("The server is not responding! Operation ignored");
				waitingReply = FALSE;
			}
			else
				waitingReply = req_resendLastMessage(asSockfd);
		}
			
	}
}



int main(int argc, char *argv[]) {
        initSignal(&terminateFS, &abortFS);
        parseArgs(argc, argv);

        /* Establish TCP connection with AS. */
        asSockfd = tcpCreateClient(connectionInfo.asip, connectionInfo.asport);
        tcpConnect(asSockfd);

        runFS();

        return 0;
}