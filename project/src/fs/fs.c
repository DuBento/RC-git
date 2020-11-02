#include "fs_aux.h"

char verbosity = FALSE;

void parseArgs(int argc, char *argv[], connectionInfo_t *info) {
		/* check the number of arguments */    
	if (argc < 1 || argc > 8 )
		_FATAL("Failed to parse arguments.\nUsage: %s [-q FSport] [-n ASIP] [-p ASport] [-v]\n", argv[0]);
	
		// else
		for (int i = 1; i < argc; i++){
				if (!strcmp(ARG_FSPORT, argv[i]) && isPortValid((const char*) argv[i+1])
						&& i+1 < argc)
						strncpy(info->fsport, argv[++i], PORT_SIZE);
				else if (!strcmp(ARG_ASIP, argv[i]) && isIPValid((const char*) argv[i+1])
						&& i+1 < argc)
						strncpy(info->asip, argv[++i], IP_SIZE);
				else if (!strcmp(ARG_ASPORT, argv[i]) && isPortValid((const char*) argv[i+1])
						&& i+1 < argc)
						strncpy(info->asport, argv[++i], PORT_SIZE);
				else if (!strcmp(ARG_VERBOS, argv[i]))
						/* activate verbose mode - flag */
						verbosity = TRUE;
				else 
						_FATAL("Failed to parse arguments.\nUsage: %s [-q FSport] [-n ASIP] [-p ASport] [-v]\n", argv[0]);
		}
		/* logs the server information (on debug mod only) */
		_LOG("Runtime settings:\nFSport\t: %s\nASIP\t: %s\nASPort\t: %s\nVerbosity\t: %d", 
				info->fsport, info->asip, info->asport, verbosity);
}


int main(int argc, char *argv[]) {
	connectionInfo_t connectionInfo = {"59053\0", "", "58053\0"}; 
	parseArgs(argc, argv, &connectionInfo);
	return 0;
}