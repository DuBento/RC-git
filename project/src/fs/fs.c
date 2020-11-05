#include "fs_aux.h"
#include "files.h"



static connectionInfo_t connectionInfo = {"\0", "59053\0", "\0", "58053\0"};
static DIR *files;
char filesPath[PATH_MAX];

bool_t verbosity = FALSE;
bool_t bRunning = TRUE;
int exitCode = 0;



/*! \brief Terminates the program on success.
 *
 *	Termination handle called by the SIGINT and SIGTERM signals.
 */
void terminateFS() {
	bRunning = FALSE;
	exitCode = 0;
}


/*! \brief Terminates the program on fatal errors.
 *
 *	Termination handle called by the SIGABRT, SIGFPE, SIGILL and SIGSEGV signals
 */
void abortFS() {
	bRunning = FALSE;
	exitCode = 1;
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




void runFS() {
	while (bRunning) {

	}
}



int main(int argc, char *argv[]) {
	initSignal(&terminateFS, &abortFS);
	parseArgs(argc, argv);

	files = initDir(argv[0], "files", filesPath);
	runFS();
	

	closedir(files);
	return 0;
}	