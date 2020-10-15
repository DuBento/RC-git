#include "common.h"

/* the information to allow communication with the servers */
typedef struct connectionInfo_t {

        char asip[IP_SIZE + 1];                 /* ip address of the autentication server - same machine if not specified */
        char asport[PORT_SIZE + 1];         /* port of the program */        
        char fsip[IP_SIZE + 1];                 /* ip address of the file server - same machine if not specified */
        char fsport[PORT_SIZE + 1];         /* port of the file server */

} connectionInfo_t;


void parseArgs(int argc, char *argv[], connectionInfo_t *info) {
        /* check the number of arguments */        
	if (argc < 1 || argc > 9 || argc % 2 != 1) {
		printf("Usage: %s [-n ASIP] [-p ASport] [-m FSIP] [-q FSport]\n", argv[0]);
		fatal("Failed to parse arguments.");
	}

    
        for (int i = 1; i < argc; i++){
                if (!strcmp(ASIPARG, argv[i]) && checkValidIp((const char *)argv[i+1])) 
                        strncpy(info->asip, argv[++i], IP_SIZE);
                else if (!strcmp(ASPORTARG, argv[i]) && checkValidPORT((const char*) argv[i+1]))
                        strncpy(info->asip, argv[++i], PORT_SIZE);
                else if (!strcmp(FSIPARG, argv[i]) && checkValidIp((const char *)argv[i+1]))
                       strncpy(info->asport, argv[++i], IP_SIZE); 
                else if (!strcmp(FSPORTARG, argv[i]) && checkValidPORT((const char *)argv[i+1]))
                       strncpy(info->asport, argv[++i], PORT_SIZE);
                else 
                        fatal("Invalid IP address format.\nPlease use dot notation.\nOr invalid PORT format. \nPlease only use unrestricted ports.");
        }

        /* logs the server information (on debug mod only) */
        _LOG("connectionInfo settings:\nASIP\t: %s\nASport\t: %s\nFSIP\t: %s\nFSport\t: %s\n", 
                info->asip, info->asport, info->fsip, info->fsport);
}



int main() {
        connectionInfo_t connectionInfo = {"", "58053\0", "193.136.138.142\0", "58011\0"};
        int asSockfd;

        parseArgs(argc, argv, &connectionInfo);
}