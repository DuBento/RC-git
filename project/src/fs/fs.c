#include "fs_aux.h"
#include "../list.h"
#include "../files.h"



static connectionInfo_t connectionInfo = {"59053\0", "\0", "58053\0"};
static DIR *files;
char filesPath[PATH_MAX];

TCPConnection_t *tcpConnection = NULL;
UDPConnection_t *udpConnection = NULL;
List_t userConnections = NULL;
List_t userRequests = NULL;

bool_t verbosity = FALSE;



/*! \brief Cleans a connection socket
 *
 *	Casts the socket to the correct type and calls the tcpDestroySocket() function.
 * 	This is used to clean the userConnections list.
 * 
 * 	\param
 */
void cleanSocket(void* socket) {
	tcpDestroySocket((TCPConnection_t*)socket);
}


/*! \brief Cleans the server and frees all the memory allocated.
 *
 *	Termination handle called by the SIGINT and SIGTERM signals.
 */
void cleanFS() {
	if (tcpConnection != NULL)		tcpDestroySocket(tcpConnection);
	if (udpConnection != NULL)		udpDestroySocket(udpConnection);
	if (userConnections != NULL)	listDestroy(userConnections, cleanSocket);
	if (userRequests != NULL)		listDestroy(userRequests, free);
	closedir(files);
}


/*! \brief Terminates the program on success.
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

	if (connectionInfo.asip[0] == '\0')
		strcpy(connectionInfo.asip, LOCAL_IP);

	// logs the server information (on debug mod only)
	_LOG("Runtime settings:\nFSport\t: %s\nASIP\t: %s\nASPort\t: %s\nVerbose\t: %d", 
			 connectionInfo.fsport, connectionInfo.asip, connectionInfo.asport, verbosity);
}



/*! \brief Adds a user to the connection list.
 *
 *  Adds a user to the connection list opens up the communication with him.
 * 
 * 	\param userConnections		the list of users currently connected.
 *  \param tcpConnection		the tcp connection that is waiting for connection requests.
 *  \param fds					a pointer to the fds.
 *  \param fdsSize				a pointer to the size of the fds.
 */
void handleUserConnection(fd_set *fds, int *fdsSize) {
	TCPConnection_t *userConnection = (TCPConnection_t*)malloc(sizeof(TCPConnection_t));
	tcpAcceptConnection(tcpConnection, userConnection);
	listInsert(userConnections, userConnection);
	FD_SET(userConnection->fd, fds);
	*fdsSize = (*fdsSize > userConnection->fd ? *fdsSize : userConnection->fd +  1);
	_LOG("Connection accepted!\n\t - IP\t: %s\n\t - PORT\t: %d\n\t - FD\t: %d", 
		tcpConnIp(userConnection), tcpConnPort(userConnection), userConnection->fd);
}


void handleUserRequest() {

}


/*! \brief Processes all the currently active requests
 *
 *  Sends the operation requests to the as everytime the waiting time expires. If the request
 *  was already sent NREQUEST_TRIES times, sends an error to the user.
 * 
 * 	\param userConnections		the list of users currently connected.
 *  \param tcpConnection		the tcp connection that is waiting for connection requests.
 *  \param fds					a pointer to the fds.
 *  \param fdsSize				a pointer to the size of the fds.
 */
void processUserRequests(const struct timeval *oldTime) {
		struct timeval newTime;
		gettimeofday(&newTime, NULL);
		float timeExpired = newTime.tv_sec - oldTime->tv_sec;

		ListIterator_t iterator = listIteratorCreate(userRequests);
		while (!listIteratorEmpty(&iterator)) {
			userRequest_t *userRequest = (userRequest_t*)listIteratorNext(&iterator);
			if (userRequest->nTries != -1 && (userRequest->timeExpired += timeExpired) > TIMEOUT) {
				if (userRequest->nTries == NREQUEST_TRIES) {
					_LOG("Maximum number of tries reached on request %s. Aborting...", userRequest->tid);
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
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(tcpConnection->fd, &fds);
	FD_SET(udpConnection->fd, &fds);
	int fdsSize = (tcpConnection->fd > udpConnection->fd ? tcpConnection->fd : udpConnection->fd) + 1;
	struct timeval tv;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;

	while (TRUE) {
		fd_set fdsTemp = fds;
		struct timeval tvTemp, currentTime;
		gettimeofday(&currentTime, NULL);
		tvTemp = tv;
		
		int selRetv = select(fdsSize, &fdsTemp, NULL, NULL, &tvTemp);
		if (selRetv == -1) {
			if (errno == EINTR)		break;	// return from signal
			_FATAL("Unable to start the select() to monitor the descriptors!\n\t - Error code: %d", errno);
		}

		// handle the new users's connections
		if (FD_ISSET(tcpConnection->fd, &fdsTemp))
			handleUserConnection(&fds, &fdsSize);

		// handle the as reply
		if (FD_ISSET(udpConnection->fd, &fdsTemp)) {
			// handle as reply
		}

		// handles the user's new requests
		ListIterator_t iterator = listIteratorCreate(userConnections);
		while (!listIteratorEmpty(&iterator)) {
			ListNode_t node = listIteratorNextNode(&iterator);
			TCPConnection_t *userConnection = (TCPConnection_t *)listValue(node);
			if (FD_ISSET(userConnection->fd, &fdsTemp)) {
				handleUserRequest();
			}
		}

		// processes the user's current requests
		processUserRequests(&currentTime);
	}
}



int main(int argc, char *argv[]) {
	initSignal(&terminateFS, &abortFS);
	parseArgs(argc, argv);

	files = initDir(argv[0], "files", filesPath);
	VERBOSE("Starting FS server...");

	tcpConnection = tcpCreateServer(NULL, connectionInfo.fsport, SOMAXCONN);
	udpConnection = udpCreateClient(connectionInfo.asip, connectionInfo.asport);
	userConnections = listCreate();
	userRequests = listCreate();
	runFS();
	
	cleanFS();
	return 0;
}	
