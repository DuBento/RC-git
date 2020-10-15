#include "common.h"
#include <cstdlib>

typedef struct connectionInfo_t {

        char asport[PORT_SIZE + 1];             /* port of the autentication server */

} connectionInfo_t;


void parseArgs(int argc, char *argv[], connectionInfo_t *info) {
        /* check the number of arguments */        
	if (argc < 1 || argc > 9 || argc % 2 != 1) {
		printf("Usage: %s [-p ASport] [-v]\n", argv[0]);
		fatal("Failed to parse arguments.");
	}
	
        // else
    for (int i = 1; i < argc; i++){
                if (!strcmp(ASPORTARG, argv[i]) && checkValidPORT((const char*) argv[i+1]))
                    strncpy(info->asport, argv[++i], PORT_SIZE);
                else if (!strcmp(VERBOSE, argv[i]))
                    /* activate verbose mode - flag? */
                else 
                        fatal("Invalid PORT format. \nPlease only use unrestricted ports.");
        }

        /* logs the server information (on debug mod only) */
        _LOG("connectionInfo settings:\nASIP\t: %s\nASport\t: %s\nFSIP\t: %s\nFSport\t: %s\n", 
                info->asip, info->asport, info->fsip, info->fsport);
}

int main() {
        /* AS makes available two server applications? Does it mean 2 process? */
        /* Default AS port. */
        connectionInfo_t connectionInfo = {"58053\0"};
        int udpSocketfd, tcpSocketfd;

        // mount UDP server socket
        //udpSocketfd = udpCreateServer(NULL, connectionInfo.asport);
        // mount TCP server socket
        //tcpSocketfd = tcpCreateServer(NULL, connectionInfo., int numConnections)

        // wait for 
        exit(EXIT_SUCCESS);
}