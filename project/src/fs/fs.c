#include "fs_aux.h"
#include "../list.h"
#include "../files.h"
#include "../common.h"



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
	if (userRequest->fileName != NULL)		free(userRequest->fileName);
	if (userRequest->data != NULL)			free(userRequest->data);
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
	userRequest->tcpConnection = (TCPConnection_t*)malloc(sizeof(TCPConnection_t));
	if (userRequest == NULL || userRequest->tcpConnection == NULL)
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


/*! \brief Checks if a request is valid according to the AS reply 
 *
 *  Finds a request that matches the AS reply, finds out if it is valid and checks
 *  if all of its parameters are well formated. If any of this goes wrong the request
 *  is removed from the server.
 */
void handleASValidationReply() {
	char buffer[BUFFER_SIZE];
	int size = udpReceiveMessage(udpConnection, NULL, buffer, BUFFER_SIZE);	
	char opcode[BUFFER_SIZE], uid[BUFFER_SIZE], tid[BUFFER_SIZE], fop, fname[BUFFER_SIZE];
	int validArgs = sscanf(buffer, "%s %s %s %c %s\n", opcode, uid, tid, &fop, fname);

	ListNode_t node = NULL;
	ListIterator_t iterator = listIteratorCreate(userRequests);
	while (!listIteratorEmpty(&iterator)) {
		ListNode_t tempNode = (ListNode_t)iterator;
		userRequest_t *tempRequest = (userRequest_t*)listIteratorNext(&iterator);
		if (!strcmp(tid, tempRequest->tid)) {
			node = tempNode;
			break;
		}
	}

	if (node == NULL) return;		// no request with the specified tid is on the list (ignores the message)
	userRequest_t *userRequest = (userRequest_t *)listValue(node);
	if (!strcmp(opcode, RESP_VLD) && !strcmp(uid, userRequest->uid) && fop == userRequest->fop && buffer[size - 1] == '\n') {
		if ((validArgs == 4 && (fop == FOP_L || fop == FOP_X)) ||
			(validArgs == 5 && (fop == FOP_R || fop == FOP_U || fop == FOP_D) && !strcmp(fname, userRequest->fileName))) 
			{
				userRequest->exeRequest(userRequest, filesPath);
				return;
			}		
	}

	char msg[BUFFER_SIZE];
	int msgSize = sprintf(msg, "%s INV\n", userRequest->replyHeader);
	tcpSendMessage(userRequest->tcpConnection, msg, msgSize);
	listRemove(userRequests, node, cleanRequest);
}


/*! \brief Fills the information about a new user request.
 *
 *  Reads the request from the user and fills the structure with it if the request is
 * 	well formated, otherwise removes the request from the server.
 * 	The user is also removed from the select's fds since only one request is allowed per
 * 	connection. 
 * 
 * 	\param 	node		the list node associated with the user request.
 *  \param 	fds			a pointer to the fds.
 *  \param 	fdsSize		a pointer to the size of the fds.
 */
void handleUserRequest(ListNode_t node, fd_set *fds, int *fdsSize) {
	userRequest_t *userRequest = (userRequest_t*)listValue(node);
	char buffer[BUFFER_SIZE];
	int size = tcpReceiveMessage(userRequest->tcpConnection, buffer, BUFFER_SIZE);

	// removes this connection from the select
	if (*fdsSize == (userRequest->tcpConnection->fd + 1))	*fdsSize--;
	FD_CLR(userRequest->tcpConnection->fd, fds);

	char opcode[BUFFER_SIZE] = { 0 }, uid[BUFFER_SIZE] = { 0 }, tid[BUFFER_SIZE] = { 0 };
	char fname[BUFFER_SIZE] = { 0 }, fsize[BUFFER_SIZE] = { 0 }, *fdata;
	int validArgs = sscanf(buffer, "%s %s %s %s %s", opcode, uid, tid, fname, fsize);

	// displays the message (on verbose mode only)
	_VERBOSE("[ %s - %d ] : %s %s %s %s", tcpConnIp(userRequest->tcpConnection), 
			tcpConnPort(userRequest->tcpConnection), opcode, uid, tid, fname);

	bool_t successOnFill;
	if (validArgs == 3 && !strcmp(opcode, REQ_LST) && buffer[size] != '\n')
		successOnFill = fillListRequest(userRequest, uid, tid);

	else if (validArgs == 4 && !strcmp(opcode, REQ_RTV) && buffer[size] != '\n')
		successOnFill = fillRetreiveRequest(userRequest, uid, tid, fname);

	else if (validArgs == 5 && !strcmp(opcode, REQ_UPL) && (fdata = findNthCharOccurence(buffer, ' ', 5)) != NULL)
		successOnFill = fillUploadRequest(userRequest, uid, tid, fname, fsize, fdata);

	else if (validArgs == 4  && !strcmp(opcode, REQ_DEL) && buffer[size] != '\n')
		successOnFill = fillDeleteRequest(userRequest, uid, tid, fname);

	else if (validArgs == 3  && !strcmp(opcode, REQ_REM) && buffer[size] != '\n')
		successOnFill = fillRemoveRequest(userRequest, uid, tid);

	else {
		tcpSendMessage(userRequest->tcpConnection, "ERR\n", 4);
		successOnFill = FALSE;
	}
		
	if (!successOnFill)
		listRemove(userRequests, node, cleanRequest);
}


/*! \brief Processes all the currently active requests
 *
 *  Sends the operation requests to the as everytime the waiting time expires. If the request
 *  was already sent NREQUEST_TRIES times, sends an error to the user.
 * 
 * 	\param oldTime		the time stamp before the select was activated.
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
					_LOG("Maximum number of tries reached on request %s. Destoying the request...", userRequest->tid);
					char msg[BUFFER_SIZE];
					int msgSize = sprintf(msg, "%s ERR\n", userRequest->replyHeader);
					tcpSendMessage(userRequest->tcpConnection, msg, msgSize);
					listRemove(userRequests, node, cleanRequest);
					return;
				}

				userRequest->nTries++;
				userRequest->timeExpired = 0;
				_LOG("Request validation update [%s] : try no%d", userRequest->tid, userRequest->nTries);
				validateRequest(udpConnection, userRequest);
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

		// handle the AS request validation reply
		if (FD_ISSET(udpConnection->fd, &fdsTemp))
			handleASValidationReply();

		// handles the user's new requests
		ListIterator_t iterator = listIteratorCreate(userRequests);
		while (!listIteratorEmpty(&iterator)) {
			ListNode_t node = (ListNode_t)iterator;
			TCPConnection_t *userConnection = ((userRequest_t*)listIteratorNext(&iterator))->tcpConnection;
			if (FD_ISSET(userConnection->fd, &fdsTemp))
				handleUserRequest(node, &fds, &fdsSize);
		}

		processUserRequests(&currentTime);	// processes the current requests stored on the server
	}
}



int main(int argc, char *argv[]) {
	initSignal(&terminateFS, &abortFS);
	parseArgs(argc, argv);

	files = initDir(argv[0], "files", filesPath);
	VERBOSE("Starting FS server...");

	tcpConnection = tcpCreateServer(NULL, connectionInfo.fsport, SOMAXCONN);
	udpConnection = udpCreateClient((connectionInfo.asip[0] == '\0' ? NULL : connectionInfo.asip), connectionInfo.asport);
	userRequests = listCreate();
	runFS();

	return 0;
}