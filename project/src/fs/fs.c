#include "fs_aux.h"
#include "../list.h"
#include "../files.h"



static connectionInfo_t connectionInfo = {"59053\0", "\0", "58053\0"};
static DIR *files;
char filesPath[PATH_MAX];

TCPConnection_t *tcpConnection = NULL;
UDPConnection_t *udpConnection = NULL;
List_t userRequests = NULL;

bool_t verbosity = FALSE;



/*! \brief Cleans a connection socket
 *
 *	Casts the socket to the correct type and calls the tcpDestroySocket() function.
 * 	This is used to clean the userConnections list.
 * 
 * 	\param socket		the socked to be destroyed.
 */
void cleanRequest(void* request) {
	userRequest_t *userRequest = (userRequest_t*)request;
	if (userRequest->tcpConnection != NULL) tcpCloseConnection(userRequest->tcpConnection);
	if (userRequest->fileName != NULL)	free(userRequest->fileName);
	if (userRequest->data != NULL)		free(userRequest->data);
	free(userRequest);
}


/*! \brief Cleans the server and frees all the memory allocated.
 *
 *	Termination handle called by the SIGINT and SIGTERM signals.
 */
void cleanFS() {
	if (tcpConnection != NULL)		tcpDestroySocket(tcpConnection);
	if (udpConnection != NULL)		udpDestroySocket(udpConnection);
	if (userRequests != NULL)		listDestroy(userRequests, cleanRequest);
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
	userRequest_t *userRequest = (userRequest_t*)malloc(sizeof(userRequest_t));
	if (userRequest == NULL)
		FATAL("Unable to allocate memory for the user request!");

	userRequest->tcpConnection = (TCPConnection_t*)malloc(sizeof(TCPConnection_t));
	if (userRequest->tcpConnection == NULL)
		FATAL("Unable to allocate memory for the user request!");
	userRequest->nTries = -1;
	userRequest->fileName = NULL;
	userRequest->data = NULL;
	
	tcpAcceptConnection(tcpConnection, userRequest->tcpConnection);
	listInsert(userRequests, userRequest);
	FD_SET(userRequest->tcpConnection->fd, fds);
	*fdsSize = (*fdsSize > userRequest->tcpConnection->fd ? *fdsSize : userRequest->tcpConnection->fd +  1);
	_LOG("Connection accepted!\n\t - IP\t: %s\n\t - PORT\t: %d\n\t - FD\t: %d", 
		tcpConnIp(userRequest->tcpConnection), tcpConnPort(userRequest->tcpConnection), userRequest->tcpConnection->fd);
}



/*! \brief Fills the information about a new user request.
 *
 *  Reads the request from the user and fills the structure with it.
 * 
 * 	\param node		the list node associated with the user request.
 */
void handleUserRequest(ListNode_t node, fd_set *fds, int *fdsSize) {
	userRequest_t *userRequest = (userRequest_t*)listValue(node);
	char buffer[BUFFER_SIZE] = { 0 };
	int size = tcpReceiveMessage(userRequest->tcpConnection, buffer, BUFFER_SIZE);
	buffer[size] = '\0';
	char opcode[BUFFER_SIZE] = { 0 }, uid[BUFFER_SIZE] = { 0 }, tid[BUFFER_SIZE] = { 0 };
	char fname[BUFFER_SIZE] = { 0 }, fsize[BUFFER_SIZE] = { 0 }, *fdata;
	int validArgs = sscanf(buffer, "%s %s %s %s %s", opcode, uid, tid, fname, fsize);

	if (*fdsSize == (userRequest->tcpConnection->fd + 1))	*fdsSize--;
	FD_CLR(userRequest->tcpConnection->fd, fds);

	if (validArgs == 3 && fillListRequest(userRequest, opcode, uid, tid)) {
		_VERBOSE("[ %s - %d ] : %c %s %s", tcpConnIp(userRequest->tcpConnection), 
			tcpConnPort(userRequest->tcpConnection), userRequest->fop, userRequest->uid, userRequest->tid);
	}
	else if (validArgs == 4 && fillRetreiveRequest(userRequest, opcode, uid, tid, fname)) {
		_VERBOSE("[ %s - %d ] : %c %s %s %s", tcpConnIp(userRequest->tcpConnection), 
			tcpConnPort(userRequest->tcpConnection), userRequest->fop, userRequest->uid, userRequest->tid, userRequest->fileName);
	}
	else if (validArgs == 5 && (fdata = findNthCharOccurence(buffer, ' ', 5)) != NULL && fillUploadRequest(userRequest, opcode, uid, tid, fname, fsize)) {
		_VERBOSE("[ %s - %d ] : %c %s %s %s", tcpConnIp(userRequest->tcpConnection), 
			tcpConnPort(userRequest->tcpConnection), userRequest->fop, userRequest->uid, userRequest->tid, userRequest->fileName);
		int fdatalen = strlen(++fdata);
		if (fdata[fdatalen] == '\n')
			fdata[fdatalen--] == '\0';
		strncpy(userRequest->data, fdata, userRequest->fileSize);
		userRequest->data[userRequest->fileSize] = '\0';
		if (userRequest->fileSize > fdatalen) {
			int dataSize = tcpReceiveMessage(userRequest->tcpConnection, &userRequest->data[fdatalen], userRequest->fileSize - fdatalen + 1);
			userRequest->data[userRequest->fileSize] = '\0';
		}			
		_LOG("File info [%lu bytes] : %s", userRequest->fileSize, userRequest->data);
	}
	else if (validArgs == 4  && fillDeleteRequest(userRequest, opcode, uid, tid, fname)) {
		_VERBOSE("[ %s - %d ] : %c %s %s %s", tcpConnIp(userRequest->tcpConnection), 
			tcpConnPort(userRequest->tcpConnection), userRequest->fop, userRequest->uid, userRequest->tid, userRequest->fileName);
	}
	else if (validArgs == 3  && fillRemoveRequest(userRequest, opcode, uid, tid)) {
		_VERBOSE("[ %s - %d ] : %c %s %s", tcpConnIp(userRequest->tcpConnection), 
			tcpConnPort(userRequest->tcpConnection), userRequest->fop, userRequest->uid, userRequest->tid);
	}
	else {
		tcpSendMessage(userRequest->tcpConnection, "ERR\n", 4);
		listRemove(userRequests, node, cleanRequest);
	}
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
			ListNode_t node = (ListNode_t)iterator;
			userRequest_t *userRequest = (userRequest_t*)listIteratorNext(&iterator);
			if (userRequest->nTries != -1 && (userRequest->timeExpired += timeExpired) > TIMEOUT) {
				if (userRequest->nTries == NREQUEST_TRIES) {
					// sends message back to the user
					_LOG("Maximum number of tries reached on request %s. Aborting...", userRequest->tid);
					listRemove(userRequests, node, cleanRequest);
					return;
				}

				_LOG("Request update [%s] : try no%d", userRequest->tid, userRequest->nTries++);
				userRequest->timeExpired = 0;
				// sends the request on to the AS server
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
		ListIterator_t iterator = listIteratorCreate(userRequests);
		while (!listIteratorEmpty(&iterator)) {
			ListNode_t node = (ListNode_t)iterator;
			TCPConnection_t *userConnection = ((userRequest_t*)listIteratorNext(&iterator))->tcpConnection;
			if (FD_ISSET(userConnection->fd, &fdsTemp)) {
				handleUserRequest(node, &fds, &fdsSize);
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
	userRequests = listCreate();
	runFS();

	return 0;
}	
