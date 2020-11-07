#include "fs_aux.h"
#include "../list.h"
#include "../files.h"



static connectionInfo_t connectionInfo = {"\0", "59053\0", "\0", "58053\0"};
static DIR *files;
char filesPath[PATH_MAX];

bool_t verbosity = FALSE;
bool_t bRunning = TRUE;
int errCode;



/*! \brief Terminates the program on success.
 *
 *	Termination handle called by the SIGINT and SIGTERM signals.
 */
void terminateFS() {
	bRunning = FALSE;
	errCode = 0;
}


/*! \brief Terminates the program on fatal errors.
 *
 *	Termination handle called by the SIGABRT, SIGFPE, SIGILL and SIGSEGV signals
 */
void abortFS() {
	bRunning = FALSE;
	errCode = 1;
}



/*! \brief Parses the execution arguments.
 *
 * 	Validates the execution arguments and sets the connection settings.
 *
 * 	\param 	argc The number of execution arguments.
 * 	\param 	argv An array with the execution arguments.
 * 	\param 	info The instance that stores the connection settings.
 */
void parseArgs(int argc, char *argv[]) {
	// checks the number of arguments   
	if (argc < 1 || argc > 8 )
		_FATAL("Failed to parse arguments.\nUsage: %s [-q FSport] [-n ASIP] [-p ASport] [-v]\n", argv[0]);
	
	for (int i = 1; i < argc; i++){
		int ipPortSwitch = 0;	
		if (!strcmp(ARG_FSPORT, argv[i]) && (ipPortSwitch = ARG_PORT) && i + 1 < argc && isPortValid(argv[i + 1]))
				strncpy(connectionInfo.fsport, argv[++i], PORT_SIZE);
		else if (!strcmp(ARG_ASIP, argv[i]) && (ipPortSwitch = ARG_IP) && i + 1 < argc && isIPValid(argv[i + 1]))
				strncpy(connectionInfo.asip, argv[++i], IP_SIZE);
		else if (!strcmp(ARG_ASPORT, argv[i]) && (ipPortSwitch = ARG_PORT) && i + 1 < argc && isPortValid(argv[i+1]))
				strncpy(connectionInfo.asport, argv[++i], PORT_SIZE);
		else if (!strcmp(ARG_VERBOS, argv[i]))
				verbosity = TRUE;
		else {
			if (ipPortSwitch == ARG_IP)
				_FATAL("Invalid " ARG_STR_IP " '%s'!""\n\t - [Usage]: "
				ARG_USAGE_IP " (x -> digit)", argv[i + 1])
			else if (ipPortSwitch == ARG_PORT)
				_FATAL("Invalid " ARG_STR_PORT " '%s'!""\n\t - [Usage]: "
				ARG_USAGE_PORT " (x -> digit)", argv[i + 1])
			else
				FATAL("Invalid execution argument flag!\n\t - [Flags]: '-q','-n', '-p', '-v'");
		}
	}

	// fills the ip's if they were not specified
	strcpy(connectionInfo.fsip, LOCAL_IP);
	if (connectionInfo.asip[0] == '\0')
		strcpy(connectionInfo.asip, LOCAL_IP);

	// logs the server information (on debug mod only)
	_LOG("Runtime settings:\nFSIP\t: %s\nFSport\t: %s\nASIP\t: %s\nASPort\t: %s\nVerbose\t: %d", 
			connectionInfo.fsip, connectionInfo.fsport, connectionInfo.asip, connectionInfo.asport, verbosity);
}



void handleUserConnection(List_t userConnections) {
	
}





void processUserRequests(const struct timeval *oldTime, List_t userRequests) {
		struct timeval newTime;
		gettimeofday(&newTime, NULL);
		float timeExpired = newTime.tv_sec - oldTime->tv_sec;

		ListIterator_t iterator = listIteratorCreate(userRequests);
		while (!listIteratorEmpty(&iterator)) {
			userRequest_t *userRequest = (userRequest_t*)listIteratorNext(&iterator);
			if (userRequest->nTries != -1 && (userRequest->timeExpired += timeExpired) > TIMEOUT) {
				if (userRequest->nTries == NREQUEST_TRIES) {
					_LOG("Maximum number of requsts reached on request %s. Aborting...", userRequest->tid);
					// send message back to the user
				}

				userRequest->exeRequest(userRequest);
			}
		}
}



/*! \brief Handles the server messages during the runtime.
 *
 *  Verifies which message was sent by the server and updates the server accordingly
 */
void runFS() {
	TCPConnection_t *tcpConnection = tcpCreateServer(connectionInfo.fsip, connectionInfo.fsport, SOMAXCONN);
	UDPConnection_t *udpConnection = udpCreateClient(connectionInfo.asip, connectionInfo.asport);
	List_t userConnections = listCreate();
	List_t userRequests = listCreate();

	while (bRunning) {
		struct timeval tv, currentTime;
		gettimeofday(&currentTime, NULL);
		tv.tv_sec = TIMEOUT;
		tv.tv_usec = 0;

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(tcpConnection->fd, &fds);
		int fdsSize = tcpConnection->fd + 1;	

		if (udpConnection != NULL) {
			FD_SET(udpConnection->fd, &fds);
			fdsSize = (fdsSize > udpConnection->fd ? fdsSize : udpConnection->fd + 1);
		}

		ListIterator_t iterator = listIteratorCreate(userConnections);
		while (!listIteratorEmpty(&iterator)) {
			int fd = ((TCPConnection_t*)listIteratorNext(&iterator))->fd;
			FD_SET(fd, &fds);
			fdsSize = (fdsSize > fd ? fdsSize : fd + 1);
		}
		


		int selRetv = select(fdsSize, &fds, NULL, NULL, &tv);
		if (selRetv == -1) {
			if (errno == EINTR)		break;	// return from signal
			_FATAL("Unable to start the select() to monitor the descriptors!\n\t - Error code: %d", errno);
		}

		// handle the new users's connections
		if (FD_ISSET(tcpConnection->fd, &fds))
			handleUserConnection(userConnections);

		// handle the as reply
		if (udpConnection != NULL && FD_ISSET(udpConnection->fd, &fds)) {
			// handle as reply
		}

		// handles the user's new requests
		iterator = listIteratorCreate(userConnections);
		while (!listIteratorEmpty(&iterator)) {
			ListNode_t node = listIteratorNextNode(&iterator);
			TCPConnection_t *userConnection = (TCPConnection_t *)listValue(node);
			if (FD_ISSET(userConnection->fd, &fds)) {
				;	// handle userRequest	
			}
		}

		// processes the user's current requests
		processUserRequests(&currentTime, userRequests);
	}
}



int main(int argc, char *argv[]) {
	initSignal(&terminateFS, &abortFS);
	parseArgs(argc, argv);

	files = initDir(argv[0], "files", filesPath);
	runFS();

	// cleans the file system and exits
	closedir(files);
	return errCode;
}	
